/****************************************************************************
 * tcp_server.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Includes all TCP-related functions for Hooli Metadata Server
****************************************************************************/

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "hdb.h"
#include "string_functions.h"
#include "hdmp_protocols.h"

#define SOCK_TYPE(s) (s == SOCK_STREAM ? "Stream" : s == SOCK_DGRAM ? \
 "Datagram" : s == SOCK_RAW ? "Raw" : "Other")
#define UNKNOWN 1 // Unknown request type
#define AUTH 2    // AUTH request
#define LIST 3    // LIST request

// Reference termination request variable from hmds
extern bool term_requested;

// Gets a list of available sockets for listening on a specified port
struct addrinfo* getServerSockAddr(const char* port);

// Binds to a listening socket given a socket list
int bindSocket(struct addrinfo* addr_list);

// Waits for a connection on a listening socket
int waitForConnection(int sockfd);

// Handles incoming data from Hooli Client
void handleConnection(int connectionfd, char* hostname);

// Parses a request message to determine if it is ready to be read in from
// the TCP receiving buffer
bool checkRequest(int* command_type, int* msg_length, char* buffer);

// Handles a request based on the specified command type
// Returns false if connection should be closed, in case of error
bool handleRequest(const int command_type, char* request, char* hostname,
  const int connectionfd);

// Handles an authorization request
// Returns false if connection should be closed
bool handleAuthRequest(char* request, char* hostname, const int connectionfd);

// Handles a file list request
// Returns false if connection should be closed
bool handleListRequest(char* request, char* hostname, const int connectionfd);

// Handles a server termination signal (i.e. Ctrl+C)
void handleTermination(int signal);

#endif