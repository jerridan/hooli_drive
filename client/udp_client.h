/****************************************************************************
 * udp_client.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions to open a UDP socket for the Hooli Client
****************************************************************************/

#ifndef UDP_CLIENT_H
#define UDP_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "udp_sockets.h"
                                                                        
// Opens a UDP socket for the client and return the socket file
// descriptor or -1 on error
int create_client_socket(char* hostname, char* port, host* server);

#endif