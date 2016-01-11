/*****************************************************************************
 * hmds_liason.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Acts as the 'liason' to the Hooli Metadata Server, handling authentication
 * and file upload list retrieval
 ****************************************************************************/

#include "hmds_liason.h"

// Handles server authentication from HMDS
char* handleServerAuth(char* hostname, char* port, char* username, char* password, int* sockfd) {
  syslog(LOG_INFO, "Connecting to server");

  // Get list of available sockets
  struct addrinfo* socket_list = getSockAddr(hostname, port);
  if(NULL == socket_list) {
    return NULL;
  }
  *sockfd = openConnection(socket_list); // Pointer to open socket

  if(-1 == *sockfd) {
    return NULL;
  }

  // Request authorization
  int auth_result = requestAuth(*sockfd, username, password);
  if(-1 == auth_result) {
    close(*sockfd);
    return NULL;
  }

  // If AUTH successful, continue
  int status_code; // Status code of response
  // Receive response from server
  char* response = receiveResponse(*sockfd, &status_code);

  // If no response, close the socket and return NULL
  if(NULL == response) {
    close(*sockfd);
    return NULL;
  }

  char* token = NULL; // Authentication token
  switch(status_code) {
    case 200:
      syslog(LOG_INFO, "Authentication successful");
      token = getToken(response);
      break;
    case 401:
      syslog(LOG_ERR, "401 Unauthorized");
      break;
    default:
      syslog(LOG_ERR, "Invalid response to authentication request");
      break;
  }
  free(response);
  return token;
}

// Gets the list of files to be uploaded from HMDS
char* getUploadList(int sockfd, char* token, hooli_file* file) {
  int list_result = requestList(sockfd, token, file);

  if(-1 == list_result) {
    return NULL;
  }

  int status_code;
  char* response = receiveResponse(sockfd, &status_code);

  if(NULL != response) {
    switch(status_code) {
      case 204:
        syslog(LOG_INFO, "No files requested");
        break;
      case 302:
        break;
      case 401:
        syslog(LOG_ERR, "Server did not authorize request");
        break;
      default:
        syslog(LOG_ERR, "Invalid response to list request");
        break;
    }
  }
  close(sockfd);
  return response;
}
























