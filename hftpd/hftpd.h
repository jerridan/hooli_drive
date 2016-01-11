/****************************************************************************
 * hftpd.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Implements the Hooli File Server
****************************************************************************/

#ifndef HFTPD_H
#define HFTPD_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include "hftpd_arg_handler.h"
#include "udp_sockets.h"
#include "hftp_messages.h"
#include "udp_server.h"

// Termination requested variable
// MUST be global in order to function! :(
bool term_requested = false;

// Handles file uploads from a client
void handleUploads(hftp_control_message* request, 
  char* db_hostname, host* client, int sockfd, char* root_dir, int timewait);

// Handles initialization messages from a client
FILE* handleInitialization(int expSequence, int sockfd, host* client, 
  char* db_hostname, char* root_dir, hftp_control_message* controlRequest, 
  char** username, int* file_size, int init_count);

// Handles the termination and TIME_WAIT states after a sequence of uploads
void handleTermination(uint8_t sequence, int sockfd, host* client, 
  int* expSequence, char* current_client_ip, int timewait);

// Creates an HFTP response given a sequence number and error code
message* createResponseMessage(uint8_t sequence, uint16_t error_code);

// Authenticates a token on the HDB server and retrieves the username
// Returns NULL on authentication failure
char* authenticateToken(uint8_t* token, char* db_hostname);

// Updates the database with the checksum of a newly uploaded file
void updateDatabaseEntry(hftp_control_message* controlRequest, char* username, 
  char* db_hostname);

// Creates the necessary directories needed for a file
// Returns true on success
bool createParentDirectories(char* filepath);

// Sends an invalid response to a request with the specified sequence
void sendInvalidResponse(uint8_t sequence, int sockfd, host* client);

// Sends a valid response to a request with the specified sequence
void sendValidResponse(uint8_t sequence, int sockfd, host* client);

// Handles a server termination signal (i.e. Ctrl+C)
void handleServerTermination(int signal);

#endif