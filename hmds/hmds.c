/****************************************************************************
 * hmds.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Generates the Hooli Metadata Server
****************************************************************************/

#include "hmds.h"

int main(int argc, char** argv) {
  //Open call to syslog
  openlog("hooli_metadata_server", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);

  // Optional arguments and defaults
  char* hostname = "localhost"; // Hostname of Redis server
  char* port = "9000";          // Port on which to listen
  int verbose_flag = 0;         // Verbose output flag

  handleOptions(argc, argv, &hostname, &port, &verbose_flag);

  // Set syslog level
  setlogmask(LOG_UPTO(verbose_flag ? LOG_DEBUG : LOG_INFO));

  // Actions for connection termination handler
  struct sigaction new_action, old_action;
  memset(&new_action, 0, sizeof(new_action));
  new_action.sa_handler = &handleTermination;
  sigaction(SIGINT, &new_action, &old_action);

  // Get list of available sockets
  struct addrinfo* socket_list = getServerSockAddr(port); // Socket list
  if(socket_list) {
    // Create a listening socket
    int sockfd = bindSocket(socket_list);
    if(-1 != sockfd) {
      // Start listening on the socket
      if(-1 == listen(sockfd, BACKLOG)) {
        syslog(LOG_ERR, "Unable to listen on bound socket");
      } else {
        syslog(LOG_INFO, "Server listening on port %s", port);
        // Wait for a connection
        int connectionfd = waitForConnection(sockfd); // Connection socket
        if(-1 != connectionfd) {
          handleConnection(connectionfd, hostname);
        }
      }
    }
    close(sockfd);
  }
  
  closelog();
  exit(EXIT_SUCCESS);
}

// Logs all user arguments via syslog in debug mode
void logArguments(char* hostname, char* port, int verbose_flag) {
  syslog(LOG_DEBUG, "Redis hostname: %s\n", hostname);
  syslog(LOG_DEBUG, "Port: %s\n", port);
  syslog(LOG_DEBUG, "Verbose: %d\n", verbose_flag);
}

// Handles a server termination signal (i.e. Ctrl+C)
void handleTermination(int signal) {
  syslog(LOG_INFO, "Termination requested");
  term_requested = true;
}