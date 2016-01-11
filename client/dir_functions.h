/*****************************************************************************
 * dir_functions.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * A set of helper functions for working with files and directories
 ****************************************************************************/

#ifndef DIR_FUNCTIONS_H
#define DIR_FUNCTIONS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <dirent.h>
 #include <errno.h>

//Returns true if a path points to a directory
bool isDirectory(char *path);

//Checks if a file name indicates a .txt file
bool isTxtFile(char *fileName);

//Checks if a directory name indicates the current or parent directory
bool isCurrentOrParentDir(char *dir);

//Combines a directory or file name with the path of its parent directory
char *getFullPathName(char *dirName, char *fileName, bool addForwardSlash);

#endif