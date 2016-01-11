/*****************************************************************************
 * dir_functions.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * A set of helper functions for working with files and directories
 ****************************************************************************/

#include "dir_functions.h"

//Returns true if a path points to a directory
bool isDirectory(char *path) {
  DIR *dir = opendir(path); //Directory pointer
  if(dir) {
    closedir(dir);
    return true;
  } else {
    //If not a directory, reset errno and return false
    errno = 0;
    return false;
  }
}

//Checks if a file name indicates a .txt file
bool isTxtFile(char *fileName) {
  //Create a short string to hold the ".txt" extension of file name
  char fileType[5];
  fileType[0] = fileName[strlen(fileName) - 4];
  fileType[1] = fileName[strlen(fileName) - 3];
  fileType[2] = fileName[strlen(fileName) - 2];
  fileType[3] = fileName[strlen(fileName) - 1];
  fileType[4] = '\0';
  //If ".txt" extension found, return true
  return(0 == strcmp(fileType, ".txt"));
}

//Checks if a directory name indicates the current or parent directory
bool isCurrentOrParentDir(char *dir) {
  return(0 == strcmp(dir, ".") || 0 == strcmp(dir, ".."));
}

//Combines a directory or file name with the path of its parent directory
char *getFullPathName(char *dirName, char *fileName, bool addForwardSlash) {
  //Full path string
  char *fullPath;
  asprintf(&fullPath, addForwardSlash ? "%s%s/" : "%s%s", dirName, fileName);
  return fullPath;
}