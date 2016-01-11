/****************************************************************************
 * client.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
****************************************************************************/

#ifndef CLIENT_H
#define CLIENT_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <zlib.h>
#include <syslog.h>

#include "structs.h"
#include "dir_functions.h"
#include "string_functions.h"
#include "argument_handler.h"
#include "hmds_liason.h"
#include "hftp_liason.h"

// Calculates the checksum of one file
int calcChecksum(char* filename);

// Logs requested files from a 302 server response
void logRequestedFiles(char* response);

// Handles the scanning of the Hooli directory
// Returns a linked list of Hooli files
hooli_file* handleHooliScan(char* hooli_dir);

// Recursively scans a specified directory, calculates file checksums, and
// adds the entries to a linked list
void scanHooliDir(char *dir, const int base_path_length, hooli_file** file,
  int* count);

// Adds a hooli_file entry into a linked list
void addHooliFile(int checksum, char* filepath, hooli_file** file, int count);

// Frees all memory allocated to a linked list of Hooli files
void freeHooliFileList(hooli_file* file);

// Logs all user arguments via syslog in debug mode
void logArguments(char* username, char* password, char* hostname, char* port, 
  char* hooli_dir, int verbose_flag, char* hftp_hostname, char* hftp_port);

// Logs all Hooli files in the linked list in debug mode
void logHooliFiles(hooli_file* file);

// Converts the upload list received from HMDP to a hooli_file structure
// Uses the original file list, copying over to new list
hooli_file* convertUploadList(char* upload_list, hooli_file* old_list, 
  int* numfiles);

#endif