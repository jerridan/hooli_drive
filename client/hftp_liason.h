/*****************************************************************************
 * hftp_liason.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Acts as the 'liason' to the Hooli File Server, sending control and data
 * messages as well as receiving response messages
****************************************************************************/
#ifndef HFTP_LIASON_H
#define HFTP_LIASON_H

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include "udp_sockets.h"
#include "hftp_messages.h"
#include "udp_client.h"
#include "structs.h"
#include "string_functions.h"

// Handles the file uploads to the server
// Returns the number of files successfully sent
int handleFileUploads(char* hooli_dir, hooli_file* uploads, char* tokenstring, 
  char* hftp_hostname, char* hftp_port, int numfiles);

// Returns the size of a file in bytes, or -1 if an error occurs
uint32_t getFileSize(char* filepath);

// Creates an HFTP Control Message and returns it as type message
message* createControlMessage(uint8_t type, uint8_t sequence, 
  uint32_t file_size, uint32_t checksum, uint8_t* token, char* filename);

// Sends a control message and receives a response
hftp_response_message* sendControlMessage(char* hftp_hostname, char* hftp_port, 
  uint8_t type, uint8_t sequence, uint32_t file_size, uint32_t checksum, 
  uint8_t* token, char* filename, host* server, int sockfd);

#endif