/****************************************************************************
 * hdmp_protocols.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Specifies protocols for all HDMP requests and responses
****************************************************************************/

#ifndef HDMP_PROTOCOLS_H
#define HDMP_PROTOCOLS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>

#include "structs.h"

// Issues an AUTH request over the specified socket connection
// Returns the number of character sent, or -1 if unsuccessful
int requestAuth(int sockfd, char* username, char* password);

// Issues a LIST request over the specified socket connection
// Returns the number of characters sent, or -1 if unsuccessful
int requestList(int sockfd, char* token, hooli_file* file);

// Issues a 200 Authentication Successful response
// Returns the number of characters sent, or -1 if unsuccessful
int respond200(int sockfd, char* token);

// Issues a 204 No files requested response
// Returns the number of characters sent, or -1 if unsuccessful
int respond204(int sockfd);

// Issues a 302 Files requested response
// Returns the number of characters sent, or -1 if unsuccessful
int respond302(int sockfd, char* req_uploads);

// Issues a 401 Unauthorized response
// Returns the number of characters sent, or -1 if unsuccessful
int respond401(int sockfd);

// Returns an HDMP header given a key and value
// Suitable for both requests and responses
char* createHeader(char* key, char* value);

// Returns an HDMP LIST request body
char* createListBody(hooli_file* file);

#endif