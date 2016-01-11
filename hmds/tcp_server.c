/****************************************************************************
 * tcp_server.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Includes all TCP-related functions for Hooli Metadata Server
****************************************************************************/

#include "tcp_server.h"

// Gets a list of available sockets for listening on a specified port
struct addrinfo* getServerSockAddr(const char* port) {
  struct addrinfo hints;    // Additional 'hints' about connection
  struct addrinfo* results; // Linked list of sockets

  // Initialize hints
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // Request IPv4 addresses
  hints.ai_socktype = SOCK_STREAM; // Request TCP sockets
  hints.ai_flags = AI_PASSIVE;     // Request a listening socket

  // Retrieve available sockets from all local IP addresses
  // If successful, retval will = 0
  int retval = getaddrinfo(NULL, port, &hints, &results);
  if(retval) {
    syslog(LOG_ERR, "%s\n", gai_strerror(retval));
    return NULL;
  }
  return results;
}

// Binds to a listening socket given a socket list
// Returns a pointer to the socket or -1 if unsuccessful
int bindSocket(struct addrinfo* addr_list) {
  struct addrinfo* addr; // Address of bound socket
  int sockfd;            // Pointer to bound socket
  char yes = '1';        // Pointer to true for re-use socket option

  // Iterate over the addresses until we successfully bind to one
  for(addr = addr_list; addr != NULL; addr = addr->ai_next) {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    // Try the next address if socket did not open
    if(-1 == sockfd) {
      continue;
    }

    // Allow the port to be re-used if currently in the TIME_WAIT state
    if(-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, 
      sizeof(int))) {
      syslog(LOG_ERR, "Unable to set re-use socket option");
      close(sockfd);
      continue;
    }

    // Bind to the socket, or try the next address on failure
    if(-1 == bind(sockfd, addr->ai_addr, addr->ai_addrlen)) {
      close(sockfd);
      continue;
    } else {
      break;
    }
  }
  freeaddrinfo(addr_list);

  // If addr is NULL, we tried every address and could not bind to any
  if(NULL == addr) {
    syslog(LOG_ERR, "Unable to bind to socket");
    return -1;
  } else {
    return sockfd;
  }
}

// Waits for a connection on a listening socket
// Returns a pointer to a connection socket or -1 if unsuccessful
int waitForConnection(int sockfd) {
  struct sockaddr_in client_addr;   // Remote IP that is connecting to us
  // Length of the remote IP structure
  unsigned int addr_len = sizeof(struct sockaddr_in); 
  char ip_address[INET_ADDRSTRLEN]; // Buffer to store human-friendly IP
  int connectionfd;                 // New connection socket

  // Wait for new connection
  connectionfd = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);

  //Make sure connection was established
  if(-1 == connectionfd) {
    syslog(LOG_ERR, "Server could not accept connection from client");
    return -1;
  }

  // Make IP human-readable and log it
  inet_ntop(client_addr.sin_family, &client_addr.sin_addr, ip_address,
    sizeof(ip_address));
  syslog(LOG_INFO, "Connection accepted from %s", ip_address);

  return connectionfd;
}

// Handles incoming data from Hooli Client
void handleConnection(int connectionfd, char* hostname) {
  int buffer_size = 4096;             // Current size of buffer
  char* buffer = malloc(buffer_size); // Initialize read buffer
  int bytes_read;                     // # bytes read into buffer

  do {
    // Receive up to 4095 bytes from client
    // MSG_PEEK flag ensures receiving buffer is not yet emptied, in case
    // of a partial message
    bytes_read = recv(connectionfd, buffer, buffer_size-1, MSG_PEEK);

    // If data was read
    if(bytes_read > 0) {
      buffer[bytes_read] = '\0';
      int msg_length = -1;  // Length of msg, if known
      int command_type = 0; // Command type 
      bool msg_complete = checkRequest(&command_type, &msg_length, buffer);
      // If command type unknown, throw error and exit
      if(UNKNOWN == command_type) {
        syslog(LOG_ERR, "Invalid request protocol");
        break;
      }
      if(msg_complete) {
        // Resize buffer back to default
        if(4096 != buffer_size) {
          buffer_size = 4096;
          buffer = realloc(buffer, buffer_size);
        }

        char request[msg_length + 1]; // Buffer for full request msg + '\0'
        // Read the full msg into the request buffer, with no flags
        recv(connectionfd, request, msg_length, 0);
        request[msg_length] = '\0';
        // Handle the request; if handled successfully, carry_on is true
        bool carry_on = handleRequest(command_type, request, hostname, 
          connectionfd);
        if(!carry_on) {
          syslog(LOG_ERR, "An error occurred, closing connection");
          break;
        }
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
  } while(bytes_read > 0 && !term_requested);
  free(buffer);
  close(connectionfd);
  syslog(LOG_INFO, "Connection terminated");
}

// Parses a request message to determine if it is ready to be read in from
// the TCP receiving buffer
bool checkRequest(int* command_type, int* msg_length, char* buffer) {
  bool ready = false; // Whether or not message is complete
  char* headers = getPrefix(buffer, "\n\n", true); // Try getting headers
  // If we can't get them, message has not been fully received
  if(NULL == headers) {
    return ready;
  }

  // Get the length of everything up to the request body
  int length_headers = strlen(headers);

  char* command = getPrefix(headers, "\n", false); // Request command

  // Set command type
  if(0 == strcmp(command, "AUTH")) {
    *command_type = AUTH;
  } else if(0 == strcmp(command, "LIST")) {
    *command_type = LIST;
  } else {
    *command_type = UNKNOWN;
  }

  switch(*command_type) {
    case AUTH:
      // Length of expected message is length of headers
      *msg_length = length_headers;
      ready = true;
      break;
    case LIST:
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
    case UNKNOWN:
      break;
  }
  free(headers);
  free(command);
  return ready;
}

// Handles a request based on the specified command type
// Returns false if connection should be closed, in case of error
bool handleRequest(const int command_type, char* request, char* hostname, 
  const int connectionfd) {
  switch(command_type) {
    case AUTH:
      return handleAuthRequest(request, hostname, connectionfd);
    case LIST:
      return handleListRequest(request, hostname, connectionfd);
    default:
      break;
  }
  return false;
}

// Handles an authorization request
// Returns false if connection should be closed
bool handleAuthRequest(char* request, char* hostname, const int connectionfd)
{
  char* uhdr = strstr(request, "Username:");         // Username header
  char* phdr = strstr(request, "Password:");         // Password header
  char* username = getPrefix(uhdr + 9, "\n", false); // Username
  char* password = getPrefix(phdr + 9, "\n", false); // Password
  syslog(LOG_INFO, "Username: %s", username);

  // Check against forbidden usernames (will conflict with database hash keys)
  if(0 == strcmp("usernames", username) || 0 == strcmp("tokens", username)) {
    syslog(LOG_ERR, "Forbidden username '%s'", username);
    free(username);
    free(password);
    if(-1 == respond401(connectionfd)) {
      return false;
    }
    return true;
  }

  // Connect to database
  hdb_connection* con = hdb_connect(hostname);
  if(NULL == con) {
    syslog(LOG_ERR, "Unable to connect to file server database");
    free(username);
    free(password);
    return false;
  }

  // Attempt user authentication, receive token
  char* token = hdb_authenticate(con, username, password);
  hdb_disconnect(con);
  free(username);
  free(password);
  if(token) {
    if(-1 == respond200(connectionfd, token)) {
      free(token);
      return false;
    }
    free(token);
    return true;
  } else {
    if(-1 == respond401(connectionfd)) {
      return false;
    }
    return true;
  }
}

// Handles a file list request
// Returns false if connection should be closed
bool handleListRequest(char* request, char* hostname, const int connectionfd)
{
  syslog(LOG_INFO, "Receiving file list");
  char* thdr = strstr(request, "Token:");         // Token header
  char* token = getPrefix(thdr + 6, "\n", false); // Token

  // Connect to database
  hdb_connection* con = hdb_connect(hostname);
  if(NULL == con) {
    syslog(LOG_ERR, "Unable to connect to file server database");
    free(token);
    return false;
  }

  // Validate token, store username
  char* username = hdb_verify_token(con, token);
  free(token);

  // Send 401 if invalid token
  if(NULL == username) {
    hdb_disconnect(con);
    free(username);
    if(-1 == respond401(connectionfd)) {
      return false;
    }
    return true;
  }

  char* uploads = NULL;                 // Uploads to request from client
  char* body = strstr(request, "\n\n"); // Pointer to body of request
  body += 2;                            // Move pointer ahead of /n/n
  char* file;                           // One requested file
  const char separator[2] = "\n";       // Char separating files in list

  file = strtok(body, separator);       // Get first file
  char* stored_checksum;                // Checksum from DB
  char* new_checksum;                   // Checksum from request

  while(NULL != file) {
    // Get stored checksum for file
    stored_checksum = hdb_file_checksum(con, username, file);
    // Get new checksum for current file (line after current filename)
    new_checksum = file;
    new_checksum = strtok(NULL, separator);

    syslog(LOG_DEBUG, "* %s, %s", file, new_checksum);

    // If new checksum does not match the one stored in the DB,
    // add it to the uploads string
    if(NULL == stored_checksum || 0 != strcmp(stored_checksum, new_checksum)) 
    {
      if(NULL == uploads) {
        asprintf(&uploads, "%s", file);
      } else {
        char* new_uploads;
        asprintf(&new_uploads, "%s\n%s", uploads, file);
        free(uploads);
        uploads = new_uploads;
      }
    }
    file = new_checksum;
    file = strtok(NULL, separator);
  }
  hdb_disconnect(con);
  free(username);

  // If no new files, respond to client with 204 message
  if(NULL == uploads) {
    if(-1 == respond204(connectionfd)) {
      return false;
    }
    return true;
    // Otherwise, respond to client with 302 message
  } else {
    if(-1 == respond302(connectionfd, uploads)) {
      return false;
    }
    // Log out which files are being requested
    syslog(LOG_INFO, "Requesting files:");
    char* req_file = strtok(uploads, separator);
    while(NULL != req_file) {
      syslog(LOG_INFO, "* %s", req_file);
      req_file = strtok(NULL, separator);
    }
    free(uploads);
    return true;
  }
}








