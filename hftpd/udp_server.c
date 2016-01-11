/****************************************************************************
 * udp_server.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions to open a UDP socket for the Hooli File Server
****************************************************************************/

#include "udp_server.h"

// Creates a UDP server socket
int createServerSocket(char* port) {

  // Get available UDP sockets
  struct addrinfo* results = get_udp_sockaddr(NULL, port, AI_PASSIVE);

  // Bind to a socket and get its file descriptor
  int sockfd = bind_socket(results);

  return sockfd;
}

// Binds to a socket given a list of available socket addresses
int bind_socket(struct addrinfo* addr_list) {
  struct addrinfo* addr;
  int sockfd;

  // Iterate through each addrinfo in the list; stop when we successfully 
  // bind to one
  for (addr = addr_list; addr != NULL; addr = addr->ai_next) {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

    // Try the next address if we couldn't open a socket
    if (sockfd == -1)
      continue;

    // Try to bind the socket to the address/port
    if (bind(sockfd, addr->ai_addr, addr->ai_addrlen) == -1) {
      // If binding fails, close the socket, and try the next address
      close(sockfd);
      continue;
    } else {
      // Otherwise, we've bound the address/port to the socket, so stop
      // processing
      break;
    }
  }

  // Free the memory allocated to the addrinfo list
  freeaddrinfo(addr_list);

  // If addr is NULL, we tried every addrinfo and weren't able to bind to any
  if (addr == NULL) {
    syslog(LOG_ERR, "Unable to bind to socket");
    return -1;
  } else {
    // Otherwise, return the socket descriptor
    return sockfd;
  }

}