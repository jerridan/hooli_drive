/*****************************************************************************
 * string_functions.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions for working with strings
 ****************************************************************************/
 
#ifndef STRING_FUNCTIONS_H
#define STRING_FUNCTIONS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

 //Returns a copy of a string
char *getStrCopy(char *str);

//Returns a copy of a string constant
char *getStrCopyOfConst(const char *str);

//Generates a random alphanumeric string
char* generateToken(int length);

// Returns a prefix of a string ending up to the first instance 
// of a specified substring, or NULL if no such prefix
char* getPrefix(char* string, char* end, bool include_end);

//Adds a forward slash to a given string, if one does not already exist
char *addForwardSlash(char *string);

#endif