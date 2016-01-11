/****************************************************************************
 * hmds.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Generates the Hooli Metadata Server
****************************************************************************/

#ifndef HMDS_H
#define HMDS_H

#define BACKLOG 25

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#include "hmds_arg_handler.h"
#include "tcp_server.h"

// Termination requested variable
bool term_requested = false;

// Logs all user arguments via syslog in debug mode
void logArguments(char* hostname, char* port, int verbose_flag);

#endif