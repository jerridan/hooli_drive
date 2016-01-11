/****************************************************************************
 * tcp_client.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Includes all TCP-related functions for Hooli client
****************************************************************************/
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "string_functions.h"

// Returns a list of socket addresses available for use, given a hostname
// and port
struct addrinfo* getSockAddr(const char* hostname, const char* port);

// Returns a pointer to a socket connection, given a list of available
// sockets
int openConnection(struct addrinfo* addr_list);

// Receives an expected response from Hooli Server to an AUTH request
// Sets the status code and returns the response message
char* receiveResponse(int sockfd, int* status_code);

// Parses a response message to determine if it is ready to be read in from
// the TCP receiving buffer
bool checkResponse(int* status_code, int* msg_length, char* buffer);

// Returns the token from an HDMP 200 response
char* getToken(char* response);

#endif