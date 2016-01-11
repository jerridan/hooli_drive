/****************************************************************************
 * hftpd.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Implements the Hooli File Server
****************************************************************************/
#include "hftpd.h"

int main(int argc, char** argv) {
  //Open call to syslog
  openlog("hooli_file_server", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);

  // Optional arguments and defaults
  char* db_hostname = "localhost"; // Hostname of Redis server
  char* port = "10000";            // Port on which to listen
  char* root_dir = "/tmp/hftpd/";  // Directory to store uploaded files
  int timewait = 10;               // Length of TIME_WAIT in seconds   
  int verbose_flag = 0;            // Verbose output flag

  handleOptions(argc, argv, &db_hostname, &port, &root_dir, &timewait, 
    &verbose_flag);

  // Set syslog level
  setlogmask(LOG_UPTO(verbose_flag ? LOG_DEBUG : LOG_INFO));

  // Actions for connection termination handler
  struct sigaction new_action, old_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = &handleServerTermination;
  sigaction(SIGINT, &new_action, &old_action);

  // Create a socket to listen on specified port
  int sockfd = createServerSocket(port);

  // Exit if socket was not created
  if(-1 == sockfd) {
    exit(EXIT_FAILURE);
  }

  while(!term_requested) {
    hftp_control_message* request; // Client's request message
    host client;                   // Client's address

    // Receive initialization message
    syslog(LOG_INFO, "Standing by for new client request");
    request = (hftp_control_message*)receive_message(sockfd, &client);

    // Handle the request to upload a file
    handleUploads(request, db_hostname, &client, sockfd, root_dir, timewait);
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}

// Sends an invalid response to a request with the specified sequence
void sendInvalidResponse(uint8_t sequence, int sockfd, host* client) {
  message* response = createResponseMessage(sequence, RESPONDINVALID);
  syslog(LOG_DEBUG, " * * Sending response to sequence %d, of size %d, with error code %d", 
    sequence, response->length, RESPONDINVALID);
  send_message(sockfd, response, client);
  free(response);
}

// Sends a valid response to a request with the specified sequence
void sendValidResponse(uint8_t sequence, int sockfd, host* client) {
  message* response = createResponseMessage(sequence, RESPONDVALID);
  syslog(LOG_DEBUG, " * * Sending response to sequence %d, of size %d, with error code %d", 
    sequence, response->length, RESPONDVALID);
  send_message(sockfd, response, client);
  free(response);
}

// Handles initialization messages from a client
FILE* handleInitialization(int expSequence, int sockfd, host* client, 
  char* db_hostname, char* root_dir, hftp_control_message* controlRequest, 
  char** username, int* file_size, int init_count) 
{
  FILE *fp = NULL; // Pointer to new file for upload

  *file_size = controlRequest->file_size; // Get file size

  syslog(LOG_DEBUG, "Received initialization request of size %d, sequence %d", 
    controlRequest->length, controlRequest->sequence);

  // If the sequence number is wrong, send an error message and return
  if(expSequence != controlRequest->sequence) {
    sendInvalidResponse(controlRequest->sequence, sockfd, client);
    return NULL;
  }

  // Get username associated with token
  *username = authenticateToken(controlRequest->token, db_hostname);
  // If the token is invalid, send an error message and return
  if(NULL == *username) {
    syslog(LOG_ERR, "Authentication failed");
    sendInvalidResponse(controlRequest->sequence, sockfd, client);
  } else {
    sendValidResponse(controlRequest->sequence, sockfd, client);

    // Root directory path must include username
    char* user_root_dir;
    asprintf(&user_root_dir, "%s%s/", root_dir, *username);

    // Get the file's name
    char filename[controlRequest->filename_len + 1];
    memcpy(filename, controlRequest->filename, controlRequest->filename_len);
    filename[controlRequest->filename_len] = '\0';
    syslog(LOG_INFO, "Receiving file %d '%s'", init_count, filename);

    char* fullFilePath; // Full system path of this file
    asprintf(&fullFilePath, "%s%s", user_root_dir, filename);
    free(user_root_dir);

    // If directory creation fails, return NULL
    if(!createParentDirectories(fullFilePath)) {
      return NULL;
    }
    fp = fopen(fullFilePath, "w+"); // Open pointer to file for writing
    free(fullFilePath);
  }
  return fp;
}

// Handles file uploads from a client
void handleUploads(hftp_control_message* controlRequest, 
  char* db_hostname, host* client, int sockfd, char* root_dir, int timewait) 
{
  int expSequence = 0; // Exepected sequence, 0 for first file
  bool clientTerminated = false; // True when client sends termination message
  char* username = NULL;         // Username of client
  int init_count = 1;            // Number of INITs received
  int file_count = 0;            // Number of files successfully received

  // Get IP address of current client
  char* currentClientIP = client->friendly_ip;

  // Receive messages while the client has not terminated uploading, and a termination
  // signal has not come in from the terminal
  while (!clientTerminated && !term_requested) {

    // Convert multi-byte values to host order
    controlRequest->filename_len = ntohs(controlRequest->filename_len);
    controlRequest->file_size = ntohl(controlRequest->file_size);
    controlRequest->checksum = ntohl(controlRequest->checksum);

    int file_size;
    if(username) {
      free(username);
    }
    FILE *fp = handleInitialization(expSequence, sockfd, client, db_hostname, 
      root_dir, controlRequest, &username, &file_size, init_count);
    free(controlRequest);

    // If a file pointer is not created, initialization failed
    if(NULL == fp) {
      syslog(LOG_ERR, " * File could not be created");
      return;
    }

    int bytes_received = 0; // Number of bytes received for current file
    // Will update user after so many bytes received
    int upload_log_point = 100000;
    while(bytes_received < file_size) {
      
      // Log progress after uploaded amount of bytes specified
      if(bytes_received >= upload_log_point) {
        // Double the log point now
        upload_log_point *= 2;
        double percent_complete = (double)bytes_received/file_size*100;
        syslog(LOG_INFO, " * %.02f%% complete (%d of %d bytes)", 
          percent_complete, bytes_received, file_size);
      }

      // Update expected sequence
      expSequence = !expSequence;

      // Get next data packet
      hftp_data_message* dataRequest = (hftp_data_message*)receive_message(sockfd, client);
      dataRequest->data_length = ntohs(dataRequest->data_length);

      char* msg_type; // Type of message just received
      switch(dataRequest->type) {
        case(DATAMSG):
          msg_type = "data";
          break;
        case(TERMMSG):
          msg_type = "termination";
          break;
        case(INITMSG):
          msg_type = "initialization";
          break;
      }
      syslog(LOG_DEBUG, " * * Received %s request of size %d, sequence %d", 
        msg_type, dataRequest->length, dataRequest->sequence);

      // Check for different client - ignore
      if(0 != strcmp(client->friendly_ip, currentClientIP)) {
        syslog(LOG_DEBUG, " * * Response not from client, ignoring");
        free(dataRequest);
        continue;
      }

      if(dataRequest->type != DATAMSG) {
        sendInvalidResponse(dataRequest->sequence, sockfd, client);
        continue;
      }

      // Check for duplicate message
      if(dataRequest->sequence != expSequence) {
        // Resend last response message
        sendValidResponse(!dataRequest->sequence, sockfd, client);
        continue;
      }

      // If an error occurs writing to the file, send an invalid response and
      // skip this file
      if(dataRequest->data_length != fwrite(dataRequest->data, sizeof(uint8_t), 
        dataRequest->data_length, fp)) {
        syslog(LOG_WARNING, " * Error writing to file");
        sendInvalidResponse(dataRequest->sequence, sockfd, client);
        break;
      }

      // Update # bytes received
      bytes_received += dataRequest->data_length;

      // Acknowledge message
      sendValidResponse(dataRequest->sequence, sockfd, client);
      free(dataRequest);
    }

    // Finished writing to file, close it
    fclose(fp);

    if(bytes_received == file_size) {
      // Update checksum of uploaded file in database
      updateDatabaseEntry(controlRequest, username, db_hostname);
      file_count++;
    }

    // Log progress
    double percent_complete = (double)bytes_received/file_size*100;
    syslog(LOG_INFO, " * %.02f%% complete (%d of %d bytes successfully transferred)", 
      percent_complete, bytes_received, file_size);

    // Get next control message from client (may be init or term)
    controlRequest = (hftp_control_message*)receive_message(sockfd, client);
    expSequence = !expSequence;

    // Check for different client - ignore
    if(0 != strcmp(client->friendly_ip, currentClientIP)) {
      syslog(LOG_DEBUG, " * * Response not from client, ignoring");
      free(controlRequest);
      continue;
    }

    // Ensure the message is not of type data
    if(DATAMSG == controlRequest->type) {
      syslog(LOG_DEBUG, " * * Received data request of size %d, sequence %d", 
        controlRequest->length, controlRequest->sequence);
      sendInvalidResponse(controlRequest->sequence, sockfd, client);
      continue;
    }

    // Check for duplicate message
    if(controlRequest->sequence != expSequence) {
      // Resend last response message
      sendValidResponse(!controlRequest->sequence, sockfd, client);
      continue;
    }

    // Check for termination message
    if(TERMMSG == controlRequest->type) {
      handleTermination(controlRequest->sequence, sockfd, client, &expSequence, 
        currentClientIP, timewait);
      clientTerminated = true;
      syslog(LOG_DEBUG, " * * Received termination request, sequence %d", 
        controlRequest->sequence);
      free(controlRequest);
      free(username);
      syslog(LOG_INFO, "%d files successfully received", file_count);
    }
  }
}

// Updates the database with the checksum of a newly uploaded file
void updateDatabaseEntry(hftp_control_message* controlRequest, char* username, 
  char* db_hostname) 
{
  // Get the file's name
  char filename[controlRequest->filename_len + 1];
  memcpy(filename, controlRequest->filename, controlRequest->filename_len);
  filename[controlRequest->filename_len] = '\0';

  // Get the checkum
  int checksum_int = (int)controlRequest->checksum;
  char* checksum;
  asprintf(&checksum, "%X", checksum_int);

  // Create the Hooli record, according the database structure
  hdb_record record;
  record.username = username;
  record.filename = filename;
  record.checksum = checksum;
  record.next = NULL;

  // Connect to the database
  hdb_connection* con = hdb_connect(db_hostname);

  // Store the file in the database
  hdb_store_file(con, &record);

  // Disconnect from the database
  hdb_disconnect(con);
  free(checksum);
}

// Handles the termination and TIME_WAIT states after a sequence of uploads
void handleTermination(uint8_t sequence, int sockfd, host* client, 
  int* expSequence, char* current_client_ip, int timewait) 
{
  sendValidResponse(sequence, sockfd, client);
  *expSequence = !*expSequence;
  syslog(LOG_INFO, "TIME_WAIT STATE... (%ds)", timewait);
  
  // TIME_WAIT state
  // Poll sockfd for the POLLIN event, in case of a duplicate message
  struct pollfd fd = {
    .fd = sockfd,
    .events = POLLIN
  };
  bool mayExit = false; // True when server may exit

  while(!mayExit) {
    int pollval = poll(&fd, 1, timewait); // Event value from poll
    if(pollval == 1 && fd.revents == POLLIN) {
      hftp_control_message* controlRequest = (hftp_control_message*)receive_message(sockfd, client);

      // Check for different client - ignore
      if(0 != strcmp(client->friendly_ip, current_client_ip)) {
        syslog(LOG_DEBUG, " * * Response not from client, ignoring");
        continue;
      }

      // Check for duplicate message
      if(controlRequest->sequence != *expSequence) {
        // Resend last response message
        sendValidResponse(!controlRequest->sequence, sockfd, client);
        continue;
      }
    } else {
      mayExit = true;
    }
  }
}

// Creates the necessary directories needed for a file
// Returns true on success
bool createParentDirectories(char* filepath) {
  // Point to last '/' in file path
  char* name_segment = strrchr(filepath, '/');
  // Get directory path
  char* dir_path = getPrefix(filepath, name_segment, false);

  char* mkdir_cmd; // Command for system mkdir call
  asprintf(&mkdir_cmd, "mkdir -p '%s'", dir_path);
  if(-1 == system(mkdir_cmd)) {
    syslog(LOG_ERR, " * Failed to create directory %s", dir_path);
    free(mkdir_cmd);
    free(dir_path);
    return false;
  } else {
    free(mkdir_cmd);
    free(dir_path);
    return true;
  }
}

// Creates an HFTP response given a sequence number and error code
message* createResponseMessage(uint8_t sequence, uint16_t error_code) {
  // Resonse message
  hftp_response_message* response = (hftp_response_message*)create_message();

  // Set response paramaters
  response->length = RESPONSELEN;
  response->type = RESPONSEMSG;
  response->sequence = sequence;
  response->error_code = htons(error_code);

  return (message*)response;
}

// Authenticates a token on the HDB server and retrieves the username
// Returns NULL on authentication failure
char* authenticateToken(uint8_t* token, char* db_hostname) {
  hdb_connection* con = hdb_connect(db_hostname);
  char* username;
  if(NULL == con) {
    syslog(LOG_ERR, "Unable to connect to file server database");
    return NULL;
  }

  char tokenstring[TOKEN_LENGTH + 1];
  memcpy(tokenstring, token, TOKEN_LENGTH);
  // Add terminating character to string
  tokenstring[TOKEN_LENGTH] = '\0';

  // Validate token, store username
  username = hdb_verify_token(con, tokenstring);
  hdb_disconnect(con);

  return username;
}

// Handles a server termination signal (i.e. Ctrl+C)
void handleServerTermination(int signal) {
  syslog(LOG_INFO, "Server shutdown requested");
  term_requested = true;
}

















