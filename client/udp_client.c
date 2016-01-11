/****************************************************************************
 * udp_client.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions to open a UDP socket for the Hooli Client
****************************************************************************/

#include "udp_client.h"

// Opens a UDP socket for the client and return the socket file
// descriptor or -1 on error
int create_client_socket(char* hostname, char* port, host* server) {
  int sockfd;            // Socket
  struct addrinfo* addr; // Socket address
  // Available socket addresses
  struct addrinfo* results = get_udp_sockaddr(hostname, port, 0);                 

  // Iterate through each addrinfo in the list;
  // stop when we successfully create a socket
  for (addr = results; addr != NULL; addr = addr->ai_next)                       
  {
    // Open a socket
    sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);      

    // Try the next address if we couldn't open a socket
    if (sockfd == -1)
      continue;

    // Copy server address and length to the out parameter 'server'
    memcpy(&server->addr, addr->ai_addr, addr->ai_addrlen);
    memcpy(&server->addr_len, &addr->ai_addrlen, sizeof(addr->ai_addrlen));

    // We've successfully created a socket; stop iterating
    break;
  }

  // Free the memory allocated to the addrinfo list
  freeaddrinfo(results);

  // If we tried every addrinfo and failed to create a socket
  if (addr == NULL) {
    syslog(LOG_INFO, "Unable to create socket");
  }

  return sockfd;
}