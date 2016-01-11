/*****************************************************************************
 * string_functions.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Helper functions for working with strings
 ****************************************************************************/

#include "string_functions.h"

// Returns a copy of a string
char* getStrCopy(char *str) {
  // Copy string to new location
  char *strCopy;
  asprintf(&strCopy, "%s", str);
  return strCopy;
}

// Returns a copy of a string constant
char* getStrCopyOfConst(const char *str) {
  // Copy string to new location
  char *strCopy;
  asprintf(&strCopy, "%s", str);
  return strCopy;
}

// Generates a random alphanumeric string
char* generateToken(int length) {
  // Token string
  char* token = malloc(length + 1);

  // Possible characters that can go into the token
  const char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  // Initialize random number generator
  srand(time(0));

  // Add random characters until the token is complete
  for(int i = 0; i < length; i++) {
    token[i] = characters[rand() % (int)(sizeof(characters) - 1)];
  }

  // Add end character to token string
  token[length] = '\0';
  return token;
}

// Returns a prefix of a string ending up to the first instance 
// of a specified substring, or NULL if no such prefix
char* getPrefix(char* string, char* end, bool include_end) {
  char* prefix = NULL;                 // Prefix string
  char* end_pos = strstr(string, end); // Position of end substring
  char* current;                       // Current position in string

  // If string is NULL, end is not in string, end is first chars in 
  // string, or end is larger than string, return NULL
  if(!string || (NULL == end_pos) || (strcmp(string, end) == 0)) {
    return NULL;
  }

  // Build the prefix until we reach the end substring
  for(current = string; current < end_pos; current++) {
    if(prefix) {
      char* new_prefix;
      asprintf(&new_prefix, "%s%c", prefix, current[0]);
      free(prefix);
      prefix = new_prefix;
    } else {
      asprintf(&prefix, "%c", current[0]);
    }
  }

  // Add the endpoint if specified
  if(include_end) {
    char* new_prefix;
    asprintf(&new_prefix, "%s%s", prefix, end);
    free(prefix);
    prefix = new_prefix;
  }
  return prefix;
}

//Adds a forward slash to a given string, if one does not already exist
char *addForwardSlash(char *string) {
  //Create a string pointer to copy string argument to
  char *newString = calloc(strlen(string) + 1, sizeof(char));
  strcpy(newString, string);
  //Add forward slash if nonexistant
  if('/' != string[strlen(string) - 1]) {
    newString = realloc(newString, strlen(string) + 2);
    strcat(newString, "/");
  }
  return newString;
}