/****************************************************************************
 * hdmp_protocols.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Specifies protocols for all HDMP requests and responses
****************************************************************************/

#include "hdmp_protocols.h"

// Issues an AUTH request over the specified socket connection
// Returns the number of characters sent, or -1 if unsuccessful
int requestAuth(int sockfd, char* username, char* password) {
  char* msg;                                       // Message to be sent
  char* command = "AUTH\n";                        // Command
  char* uhdr = createHeader("Username", username); // Username header
  char* phdr = createHeader("Password", password); // Password header
  asprintf(&msg, "%s%s%s\n", command, uhdr, phdr);
  // Send request
  syslog(LOG_DEBUG, "Sending credentials");
  int result = send(sockfd, msg, strlen(msg), 0);
  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "Authentication request not sent");
  }
  free(msg);
  free(uhdr);
  free(phdr);
  return result;
}

// Issues a LIST request over the specified socket connection
// Returns the number of characters sent, or -1 if unsuccessful
int requestList(int sockfd, char* token, hooli_file* file) {
  syslog(LOG_INFO, "Uploading file list");
  char* msg;                                         // Message to be sent
  char* command = "LIST\n";                          // Command
  char* body = createListBody(file);                 // Request body
  char* thdr = createHeader("Token", token);         // Token header
  char* b_length;                                    // Body length
  // -1 to account for newline after headers
  asprintf(&b_length, "%zu", strlen(body) - 1);
  // Body length header
  char* lhdr = createHeader("Length", b_length);
  // Note that extra '\n' after headers added by body
  asprintf(&msg, "%s%s%s%s", command, thdr, lhdr, body);
  // Send request
  int result = send(sockfd, msg, strlen(msg), 0);
  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "File list not sent\n");
  }
  free(msg);
  free(body);
  free(b_length);
  free(thdr);
  free(lhdr);
  return result;
}

// Issues a 200 Authentication Successful response
// Returns the number of characters sent, or -1 if unsuccessful
int respond200(int sockfd, char* token) {
  char* msg;                                         // Message to be sent
  char* status = "200 Authentication successful\n";  // Status
  char* thdr = createHeader("Token", token);         // Token header
  asprintf(&msg, "%s%s\n", status, thdr);

  // Send response
  int result = send(sockfd, msg, strlen(msg), 0);

  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "200 Authentication response not sent");
  } else {
    syslog(LOG_DEBUG, "Authentication successful");
  }
  free(msg);
  free(thdr);
  return result;
}

// Issues a 204 No files requested response
// Returns the number of characters sent, or -1 if unsuccessful
int respond204(int sockfd) {
  char* msg;                                  // Message to be sent
  char* status = "204 No files requested\n";  // Status
  asprintf(&msg, "%s\n", status);

  // Send response
  syslog(LOG_DEBUG, "Sending 204 response");
  int result = send(sockfd, msg, strlen(msg), 0);

  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "204 response not sent");
  } else {
    syslog(LOG_INFO, "No files requested");
  }
  free(msg);
  return result;
}

// Issues a 302 Files requested response
// Returns the number of characters sent, or -1 if unsuccessful
int respond302(int sockfd, char* req_uploads) {
  char* msg;                                      // Message to be sent
  char* status = "302 Files requested\n";         // Status
  char* b_length;                                 // Length of body
  asprintf(&b_length, "%zu", strlen(req_uploads));
  char* lhdr = createHeader("Length", b_length);  // Length header
  asprintf(&msg, "%s%s\n%s", status, lhdr, req_uploads);
  // Send response
  int result = send(sockfd, msg, strlen(msg), 0);

  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "302 response not sent");
  }
  free(msg);
  free(b_length);
  free(lhdr);
  return result;
}

// Issues a 401 Unauthorized response
// Returns the number of characters sent, or -1 if unsuccessful
int respond401(int sockfd) {
  char* msg;                            // Message to be sent
  char* status = "401 Unauthorized\n";  // Status
  asprintf(&msg, "%s\n", status);

  // Send response
  int result = send(sockfd, msg, strlen(msg), 0);

  // Log error if send unsuccessful
  if(-1 == result) {
    syslog(LOG_ERR, "401 Unauthorized response not sent");
  } else {
    syslog(LOG_INFO, "401 Unauthorized");
  }
  free(msg);
  return result;
}

// Returns an HDMP header given a key and value
// Suitable for both requests and responses
char* createHeader(char* key, char* value) {
  char* header;
  asprintf(&header, "%s:%s\n", key, value);
  return header;
}

// Returns an HDMP LIST request body
char* createListBody(hooli_file* file) {
  char* body = NULL; // Body of list request
  // Iteratively concatenate body with next file list entry
  while(file) {
    char* entry;   // Current entry
    if(body) {
      asprintf(&entry, "%s\n%s\n%X", body, file->filepath, file->checksum);
      free(body);
    } else {
      asprintf(&entry, "\n%s\n%X", file->filepath, file->checksum);
    }
    body = entry;
    file = file->next;
  }
  return body;
}
