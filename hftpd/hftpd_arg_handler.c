/****************************************************************************
 * hftpd_arg_handler.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 * 
 * Handles all required and optional arguments for Hooli File Server
****************************************************************************/

#include "hftpd_arg_handler.h"

// Handles option assignments via getopt_long
void handleOptions(int argc, char** argv, char** db_hostname, char** port,
  char** root_dir, int* timewait, int* verbose_flag) {
  // Iterate over the passed in options
  while(1) {
    // Define possible long options
    struct option long_options[] = {
      {"redis",    required_argument, 0,           'r'},
      {"port",     required_argument, 0,           'p'},
      {"dir",      required_argument, 0,           'd'},
      {"timewait", required_argument, 0,           't'},
      {"verbose",  no_argument,       verbose_flag, 1 },
      {0, 0, 0, 0}
    };

    int c;                // Current option returned
    int option_index = 0; // Long option index

    c = getopt_long(argc, argv, "r:p:d:t:v", long_options, &option_index);
    
    // If we reach the end of the options, stop iterating
    if(-1 == c) {
      break;
    }

    switch(c) {
      case 'r':
        *db_hostname = optarg;
        break;
      case 'p':
        *port = optarg;
        break;
      case 'd':
        *root_dir = addForwardSlash(optarg);
        break;
      case 't':
        *timewait = (int)strtol(optarg, (char**)NULL, 10);
        break;
      case 'v':
        *verbose_flag = 1;
        break;
      case '?':
        // Error message will be printed by getopt_long
        closelog();
        exit(EXIT_FAILURE);
    }
  }
}