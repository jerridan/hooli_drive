/****************************************************************************
 * structs.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Struct definitions required throughout Hooli client and server
****************************************************************************/

#ifndef STRUCTS_H
#define STRUCTS_H

// A node representing a file in the Hooli directory and its checksum
typedef struct hooli_file {
  char* filepath;          // File path relative to Hooli directory
  int checksum;            // Checksum of file
  struct hooli_file* next; // Pointer to next hooli_file in linked list
} hooli_file;

#endif

