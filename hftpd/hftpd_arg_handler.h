/****************************************************************************
 * hftpd_arg_handler.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 * 
 * Handles all required and optional arguments for Hooli File Server
****************************************************************************/

#ifndef HFTPD_ARG_HANDLER_H
#define HFTPD_ARG_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <syslog.h>
#include "string_functions.h"

// Handles option assignments via getopt_long
void handleOptions(int argc, char** argv, char** db_hostname, char** port,
  char** root_dir, int* timewait, int* verbose_flag);

#endif