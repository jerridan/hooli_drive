/*****************************************************************************
 * hmds_liason.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Acts as the 'liason' to the Hooli Metadata Server, handling authentication
 * and file upload list retrieval
 ****************************************************************************/

#ifndef HMDS_LIASON_H
#define HMDS_LIASON_H

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include "tcp_client.h"
#include "hdmp_protocols.h"

// Handles server authentication from HMDS
char* handleServerAuth(char* hostname, char* port, char* username, char* password, int* sockfd);

// Gets the list of files to be uploaded from HMDS
char* getUploadList(int sockfd, char* token, hooli_file* file);

#endif