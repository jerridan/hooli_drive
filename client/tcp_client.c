/****************************************************************************
 * tcp_client.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Includes all TCP-related functions for Hooli client
****************************************************************************/

#include "tcp_client.h"

// Returns a list of socket addresses available for use, given a hostname
// and port
struct addrinfo* getSockAddr(const char* hostname, const char* port) {
  struct addrinfo hints;    // Additional 'hints' about connection
  struct addrinfo* results; // Linked list of sockets

  // Initialize hints, request IPv4 and TCP
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // Retrieve available sockets
  // If successful, retval will = 0
  int retval = getaddrinfo(hostname, port, &hints, &results);
  if(retval) {
    // Convert error to human-readable string, log, and exit
    syslog(LOG_ERR, "%s\n", gai_strerror(retval));
    return NULL;
  }
  return results;
}

// Returns a pointer to a socket connection, given a list of available
// sockets, or -1 if connection cannot be opened
int openConnection(struct addrinfo* addr_list) {
  struct addrinfo* addr; // Current socket address
  int sockfd;            // Pointer to open socket

  for(addr = addr_list; addr != NULL; addr = addr->ai_next) {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    // If unsuccessful, try the next address
    if(-1 == sockfd) {
      continue;
    }

    // Stop iterating if we're able to connect to the server
    if(-1 != connect(sockfd, addr->ai_addr, addr->ai_addrlen)) {
      break;
    }
  }
  // Free memory allocated to the addrinfo list
  freeaddrinfo(addr_list);

  // Log error and exit if connection failed
  if(NULL == addr) {
    syslog(LOG_ERR, "Unable to connect to server");
    return -1;
  } else {
    syslog(LOG_DEBUG, "Connection opened");
    return sockfd;
  }
}

// Receives an expected response from Hooli Server to an AUTH request
// Sets the status code and returns the response message
char* receiveResponse(int sockfd, int* status_code) {
  int buffer_size = 4096;
  char* buffer = malloc(4096); // Initialize read buffer
  int bytes_read;              // # bytes read into buffer
  char* response = NULL;       // Response received
  bool msg_complete = false;   // Whether or not message is complete
  do {
    // Receive up to 54 bytes from server
    // MSG_PEEK flag ensures receiving buffer is not yet emptied, in case
    // of a partial message
    bytes_read = recv(sockfd, buffer, buffer_size - 1, MSG_PEEK);
    if(bytes_read > 0) {
      buffer[bytes_read] = '\0';
      int msg_length = -1;  // Length of msg
      msg_complete = checkResponse(status_code, &msg_length, buffer);
      if(msg_complete) {
        // Resize buffer back to default
        if(4096 != buffer_size) {
          buffer_size = 4096;
          buffer = realloc(buffer, buffer_size);
        }
        response = malloc(msg_length + 1);
        // Read the full msg into the response buffer, with no flags
        recv(sockfd, response, msg_length, 0);
        response[msg_length] = '\0';
        break;
      } else {
        if(4095 == bytes_read) {
          // If we've read 4095 bytes and still don't know the length of the
          // msg, the header is extremely large or the message is breaking
          // HMDP protocol - log an error and close the connection
          if(-1 == msg_length) {
            syslog(LOG_ERR, "Invalid request, header exceeded 4095 bytes");
            break;
          }
        }
        // Resize buffer as required by message
        if(-1 != msg_length) {
          buffer_size = msg_length + 1;
          buffer = realloc(buffer, buffer_size);
        }
      }
    }
  } while(bytes_read > 0);
  if(0 == bytes_read) {
    syslog(LOG_ERR, "Server terminated connection");
  }
  free(buffer);
  return response;
}

// Parses a response message to determine if it is ready to be read in from
// the TCP receiving buffer
bool checkResponse(int* status_code, int* msg_length, char* buffer) {
  bool ready = false; // Whether or not message is complete
  char* headers = getPrefix(buffer, "\n\n", true); // Try getting headers
  if(NULL == headers) {
    return ready;
  }

  // Get the length of everything up to the response body
  int length_headers = strlen(headers);

  const int STATUS_LENGTH = 4; // Length of status code
  char status[STATUS_LENGTH]; // Response status
  for (int i = 0; i < STATUS_LENGTH; i++) {
    status[i] = headers[i];
  }
  status[STATUS_LENGTH] = '\0';

  // Get status code and determine if message is ready
  *status_code = atoi(status);
  switch(*status_code) {
    case 200:
      *msg_length = length_headers;
      ready = true;
      break;
    case 204:
      *msg_length = length_headers;
      ready = true;
      break;
    case 302:
      // If not yet known, get length of msg expected
      if(-1 == *msg_length) {
        char* length_hdr = strstr(headers, "Length:"); // Length header
        // Value in length header
        char* length_num = getPrefix(length_hdr + 7, "\n", false);
        int body_length = atoi(length_num);            // Length value as int
        *msg_length = length_headers + body_length;    // Update msg length
        free(length_num);
      }
      // If the buffer contains enough bytes to include the whole msg,
      // return true
      if(strlen(buffer) >= *msg_length) {
        ready = true;
      }
      break;
    case 401:
      *msg_length = length_headers;
      ready = true;
      break;
    default:
      // In case of unknown status code, pass the headers back and let
      // client determine course of action
      *msg_length = length_headers;
      ready = true;
      break;
  }
  free(headers);
  return ready;
}

// Returns the token from an HDMP 200 response
char* getToken(char* response) {
  char* token_hdr = strstr(response, "Token:"); // Token header
  return getPrefix(token_hdr + 6, "\n", false); // Return token value
}