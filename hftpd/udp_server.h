/****************************************************************************
 * udp_server.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions to open a UDP socket for the Hooli File Server
****************************************************************************/

#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "hdb.h"
#include "udp_sockets.h"

// Creates a UDP server socket
int createServerSocket(char* port);

// Binds to a socket given a list of available socket addresses
int bind_socket(struct addrinfo* addr_list);

#endif