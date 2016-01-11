/****************************************************************************
 * udp_sockets.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jeff Shantz, Jerridan Quiring
 *
 * A library for sending and receiving messages over a UDP network
****************************************************************************/

#include "udp_sockets.h"

// Returns a list of available UDP sockets
struct addrinfo* get_udp_sockaddr(const char* node, const char* port, int flags)
{
  struct addrinfo hints;    // Address hint parameters
  struct addrinfo* results; // Available UDP socket addresses
  int retval;               // Return value of getaddrinfo

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_family = AF_INET;      // Return socket addresses for IPv4 addresses
  hints.ai_socktype = SOCK_DGRAM; // Return UDP sockets
  hints.ai_flags = flags;         // Socket addresses should be listening sockets

  retval = getaddrinfo(node, port, &hints, &results);

  if(retval != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
  }

  return results;
}

// Allocates memory for a message and returns a pointer to it
message* create_message() {
  return (message*)malloc(sizeof(message));
}

// Receives a message given a source and socket
message* receive_message(int sockfd, host* source) {
  message* msg = create_message();

  // Length of the remote IP structure
  source->addr_len = sizeof(source->addr);

  // Read message, storing its contents in msg->buffer, and
  // the source address in source->addr
  msg->length = recvfrom(sockfd, msg->buffer, sizeof(msg->buffer), 0, (struct sockaddr*)&source->addr, &source->addr_len);

  // If a message was read
  if(msg->length > 0) {
    // Convert the source address to human-readable form and store
    inet_ntop(source->addr.sin_family, &source->addr.sin_addr, source->friendly_ip, sizeof(source->friendly_ip));

    // Return msg received
    return msg;  
  } else {
    // Otherwise, free memory and return NULL
    free(msg);
    return NULL;
  }
}

// Sends a message given a destination and socket
int send_message(int sockfd, message* msg, host* dest) {
  return sendto(sockfd, msg->buffer, msg->length, 0, (struct sockaddr*)&dest->addr, dest->addr_len);
}








