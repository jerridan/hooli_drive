/****************************************************************************
 * udp_sockets.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jeff Shantz, Jerridan Quiring
 *
 * A library for sending and receiving messages over a UDP network
****************************************************************************/

#ifndef UDP_SOCKETS_H
#define UDP_SOCKETS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

// Max amount of data per UDP datagram
#define UDP_MSS 1472

// Returns a list of available UDP sockets
struct addrinfo* get_udp_sockaddr(const char* node, const char* port, int flags);

// Struct message for storing a message and its length
typedef struct {
  int length;              // Length of message
  uint8_t buffer[UDP_MSS]; // Message buffer
} message;

// Stores the packed, encoded form of the source address and its length
// Stores source address in human-readable form also
typedef struct {
  struct sockaddr_in addr;           // Source address
  socklen_t addr_len;                // Source address length
  char friendly_ip[INET_ADDRSTRLEN]; // Source human-readable IP
} host;

// Allocates memory for a message and returns a pointer to it
message* create_message();

// Receives a message given a source and socket
message* receive_message(int sockfd, host* source);

// Sends a message given a destination and socket
int send_message(int sockfd, message* msg, host* dest);

#endif