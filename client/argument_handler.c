/****************************************************************************
 * argument_handler.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 * 
 * Handles all required and optional arguments for Hooli client
****************************************************************************/

#include "argument_handler.h"

// Handles option assignments via getopt_long
void handleOptions(int argc, char** argv, char** hostname, char** port, 
  char** hooli_dir, int* verbose_flag, char** hftp_hostname, char** hftp_port) {
  // Iterate over the passed in options
  while(1) {
    // Define possible long options
    struct option long_options[] = {
      {"server",  required_argument, 0,            's'},
      {"port",    required_argument, 0,            'p'},
      {"dir",     required_argument, 0,            'd'},
      {"verbose", no_argument,       verbose_flag,  1 },
      {"fserver", required_argument, 0,            'f'},
      {"fport",   required_argument, 0,            'o'},
      {0, 0, 0, 0}
    };

    int c;                // Current option returned
    int option_index = 0; // Long option index

    c = getopt_long(argc, argv, "s:p:d:vf:o:", long_options, &option_index);
    
    // If we reach the end of the options, stop iterating
    if(-1 == c) {
      break;
    }

    switch(c) {
      case 's':
        *hostname = optarg;
        break;
      case 'p':
        *port = optarg;
        break;
      case 'd':
        free(*hooli_dir);
        *hooli_dir = addForwardSlash(optarg);
        break;
      case 'v':
        *verbose_flag = 1;
        break;
      case 'f':
        *hftp_hostname = optarg;
        break;
      case 'o':
        *hftp_port = optarg;
        break;
      case '?':
        // Error message will be printed by getopt_long
        closelog();
        exit(EXIT_FAILURE);
    }
  }
}

// Handles username and password assignments
void handleCredentials(int argc, char** argv, char** username,
  char** password) {
  // If the user did not enter in a username and password, print an error
  // and exit
  // Note: optind will point to the next argument
  if((optind + 2) > argc) {
    syslog(LOG_ERR, "A username and password must be entered\nUsage: %s [options] username password\n", argv[0]);
    closelog();
    exit(EXIT_FAILURE);
  }

  // Get username and password, which should be entered after other options
  *username = argv[optind];
  *password = argv[optind+1];
}