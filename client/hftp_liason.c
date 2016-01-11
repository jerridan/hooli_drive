/*****************************************************************************
 * hftp_liason.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Acts as the 'liason' to the Hooli File Server, sending control and data
 * messages as well as receiving response messages
****************************************************************************/
#include "hftp_liason.h"

// Handles the file uploads to the server
// Returns the number of files successfully sent
int handleFileUploads(char* hooli_dir, hooli_file* uploads, char* tokenstring, 
  char* hftp_hostname, char* hftp_port, int numfiles) {

  int filesUploaded = 0;    // Number of files successfully uploaded
  int current_file_num = 0; // Number of current file (out of numfiles)

  host server;           // Address of the server
  // Create a socket to listen on specified port
  int sockfd = create_client_socket(hftp_hostname, hftp_port, &server);
  int pollval;           // Polling event value

  uint8_t sequence = 0;  // Current message sequence
  uint8_t token[TOKEN_LENGTH];
  memcpy(token, tokenstring, TOKEN_LENGTH);
  FILE* fp;              // File pointer

  while(uploads) {
    current_file_num++;
    syslog(LOG_INFO, "Uploading file %d of %d: '%s'", current_file_num, 
      numfiles, uploads->filepath);

    // Get absolute path to file
    char* file_abs_path;
    asprintf(&file_abs_path, "%s%s", hooli_dir, uploads->filepath);
    // Get the size of the file
    uint32_t file_size;
    // If there's an error, go to the next file
    if(-1 == (file_size = getFileSize(file_abs_path))) {
      free(file_abs_path);
      continue;
    }

    uint32_t bytes_uploaded = 0; // Number of bytes of file uploaded

    // Send an initialization msg
    hftp_response_message* response = NULL;
    while(NULL == response) {
      response = sendControlMessage(hftp_hostname, 
      hftp_port, INITMSG, sequence, file_size, (uint32_t)uploads->checksum, 
      token, uploads->filepath, &server, sockfd);
    }

    response->error_code = ntohs(response->error_code);
    syslog(LOG_DEBUG, " * * Received ACK for sequence %d, error code %d", 
        response->sequence, response->error_code);

    // If initialization failed, go to next file
    if(1 == response->error_code) {
      close(sockfd);
      free(response);
      break;
    }
    free(response);

    char* serverIP = server.friendly_ip;  // Note the IP of the server
    sequence = !sequence;                 // Update sequence
    fp = fopen(file_abs_path, "r");
    free(file_abs_path);

    // Will update user after so many bytes uploaded
    int upload_log_point = 100000;
    
    // Upload the file in data messages
    while(bytes_uploaded < file_size) {
      // Log progress after uploaded amount of bytes specified
      if(bytes_uploaded >= upload_log_point) {
        // Double the log point now
        upload_log_point *= 2;
        double percent_complete = (double)bytes_uploaded/file_size*100;
        syslog(LOG_INFO, " * %.02f%% complete (%d of %d bytes)", 
          percent_complete, bytes_uploaded, file_size);
      }

      hftp_data_message* request = (hftp_data_message*)create_message();
      uint16_t bytes_read = fread(request->data, 1, MAX_DATA_LENGTH, fp);

      // If unable to read the file, log an error and move on to next file
      if(!bytes_read) {
        syslog(LOG_WARNING, "Unable to read file '%s'", uploads->filepath);
        fclose(fp);
        break;
      }

      request->type = DATAMSG;
      request->sequence = sequence;
      request->data_length = htons(bytes_read);
      // Length is number bytes read from file + 4 bytes of headers
      request->length = bytes_read + 4;

      // We will poll sockfd for the POLLIN event
      struct pollfd fd = {
        .fd = sockfd,
        .events = POLLIN
      };

      bool resend = true;          // True when the message should be (re)sent
      bool ignoreResponse = false; // True when the wrong ack is received
      while(resend) {
        if(!ignoreResponse) {
          syslog(LOG_DEBUG, " * * Data request, sequence %d, of size %d", 
            sequence, request->length);
          send_message(sockfd, (message*)request, &server);
        }
        // Poll the socket for 10 seconds
        pollval = poll(&fd, 1, 10000);
        if(pollval == 1 && fd.revents == POLLIN) {
          response = (hftp_response_message*)receive_message(sockfd, &server);

          // Check that response is from server, ignore otherwise
          if(0 != strcmp(server.friendly_ip, serverIP)) {
            syslog(LOG_DEBUG, " * * Response not from server, ignoring");
            ignoreResponse = true;
            // free(response);
            continue;
          }

          // Check response sequence
          // If wrong sequence, ignore response
          if(response->sequence == sequence) {
            resend = false;
            sequence = !sequence;
            bytes_uploaded += bytes_read;
            free(request);
          } else {
            ignoreResponse = true;
            free(response);
          }         
        } else {
          ignoreResponse = false;
        }
      }

      response->error_code = ntohs(response->error_code);

      syslog(LOG_DEBUG, " * * Received ACK for sequence %d, error code %d", 
        response->sequence, response->error_code);

      // If an error occured, move to the next file
      if(response->error_code) {
        free(response);
        fclose(fp);
        break;
      }
      free(response);
    }

    // Log progress complete
    double percent_complete = file_size/bytes_uploaded*100;
    syslog(LOG_INFO, " * %.02f%% complete (%d of %d bytes successfully transferred)", 
      percent_complete, bytes_uploaded, file_size);

    if(bytes_uploaded == file_size) {
      filesUploaded ++;
    }
    fclose(fp);
    uploads = uploads->next;
  }

  // Send termination message
  hftp_response_message* response = NULL;
  while(NULL == response) {
    response = sendControlMessage(hftp_hostname, 
    hftp_port, TERMMSG, sequence, 0, 0, token, "nofile", &server, sockfd);
  }

  response->error_code = ntohs(response->error_code);
  syslog(LOG_DEBUG, " * * Received ACK for sequence %d, error code %d", 
        response->sequence, response->error_code);
  free(response);

  close(sockfd);
  return filesUploaded;
}

// Returns the size of a file in bytes, or -1 if an error occurs
uint32_t getFileSize(char* filepath) {
  struct stat file_status;
  uint32_t size;

  if(stat(filepath, &file_status) == 0) {
    size = (uint32_t)file_status.st_size;
  } else {
    syslog(LOG_WARNING, "Error getting size of '%s', skipping", filepath);
    size = -1;
  }
  return size;
}

// Creates an HFTP Control Message and returns it as type message
message* createControlMessage(uint8_t type, uint8_t sequence, 
  uint32_t file_size, uint32_t checksum, uint8_t* token, char* filename) 
{
  hftp_control_message* msg = (hftp_control_message*)create_message();

  // Get filename length
  uint16_t filename_len = strlen(filename);

  // Store the message variables, convert multi-byte values to network order
  msg->type = type;
  msg->sequence = sequence;
  msg->filename_len = htons(filename_len);
  msg->file_size = htonl(file_size);
  msg->checksum = htonl(checksum);
  // Store filename and token as array of bytes
  memcpy(msg->token, token, TOKEN_LENGTH);
  memcpy(msg->filename, filename, filename_len);

  // Message has 28 bytes of headers, plus the length of the filename
  msg->length = filename_len + 28;

  return (message*)msg;
}

// Sends a control message and receives a response
hftp_response_message* sendControlMessage(char* hftp_hostname, char* hftp_port, 
  uint8_t type, uint8_t sequence, uint32_t file_size, uint32_t checksum, 
  uint8_t* token, char* filename, host* server, int sockfd) {

  hftp_response_message* response; // Response returned by the server
  int pollval;                     // Return value of poll

  // Encode the message to be sent
  message* request = createControlMessage(type, sequence, file_size, checksum, 
    token, filename);

  // Send it and free its memory
  if(INITMSG == type) {
    syslog(LOG_DEBUG, " * * Initialization request, sequence %d, of size %d", 
    sequence, request->length);
  } else {
    syslog(LOG_DEBUG, " * * Termination request, sequence %d, of size %d", 
    sequence, request->length);
  }
  
  send_message(sockfd, request, server);
  free(request);

  // We will poll sockfd for the POLLIN event
  struct pollfd fd = {
    .fd = sockfd,
    .events = POLLIN
  };

  // Poll the socket for 10 seconds
  pollval = poll(&fd, 1, 10000);

  if(pollval == 1 && fd.revents == POLLIN) {
    // Read the response from the server and convert the error code to
    // host order
    response = (hftp_response_message*)receive_message(sockfd, server);
    response->error_code = ntohs(response->error_code);

    // If response is not for this message, ignore it
    if(response->sequence != sequence) {
      free(response);
      return NULL;
    }
  } else {
    response = NULL;
  }

  return response;
}
