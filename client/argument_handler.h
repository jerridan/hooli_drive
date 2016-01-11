/****************************************************************************
 * argument_handler.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 * 
 * Handles all required and optional arguments for Hooli client
****************************************************************************/

#ifndef ARGUMENT_HANDLER_H
#define ARGUMENT_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <syslog.h>

#include "string_functions.h"

// Handles option assignments via getopt_long
void handleOptions(int argc, char** argv, char** hostname, char** port, 
  char** hooli_dir, int* verbose_flag, char** hftp_hostname, char** hftp_port);

// Handles username and password assignments
void handleCredentials(int argc, char** argv, char** username, 
  char** password);

#endif