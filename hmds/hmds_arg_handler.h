/****************************************************************************
 * hmds_arg_handler.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 * 
 * Handles all required and optional arguments for Hooli Metadata Server
****************************************************************************/

#ifndef HMDS_ARG_HANDLER_H
#define HMDS_ARG_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <syslog.h>

// Handles option assignments via getopt_long
void handleOptions(int argc, char** argv, char** hostname, char** port,
  int* verbose_flag);

#endif