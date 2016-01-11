/*****************************************************************************
 * hdb.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Implements the functions of libhdb
 ****************************************************************************/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hiredis/hiredis.h>

#include "hdb.h"
#include "redis_queries.h"

// Connect to the specified Hooli database server, returning the initialized
// connection.
hdb_connection* hdb_connect(const char* server) {
  redisContext *con; //Declare redis context variable
  const int port = 6379; //Connection port of redis server
  struct timeval timeout = {2, 0}; //2 sec timeout

  //Connect to redis server
  con = redisConnectWithTimeout(server, port, timeout);
  //Check if connection failed
  if(NULL == con || con->err) {
    if(con) {
      fprintf(stderr, "Connection error: %s\n", con->errstr);
      redisFree(con);
    } else {
      fprintf(stderr, "Connection error: unable to connect to specified server '%s'\n", server);
    }
    return NULL;
  }
  //Return connection
  return *(hdb_connection**)&con;
}

// Disconnect from the Hooli database server.
void hdb_disconnect(hdb_connection* con) {
  redisFree((redisContext*)con);
}

// Store a file record in the Hooli database.
void hdb_store_file(hdb_connection* con, hdb_record* record) {
  redis_hset(con, record->username, record->filename, record->checksum);
}

// Remove a file record from the Hooli database.
int hdb_remove_file(hdb_connection* con, const char* username,
  const char* filename) {
  //Returns number of records deleted
  return redis_hdel(con, username, filename);
}

// If the specified file is found in the Hooli database, return its checksum.
// Otherwise, return NULL.
char* hdb_file_checksum(hdb_connection* con, const char* username,
  const char* filename) {
  return redis_hget(con, username, filename);
}

// Return a count of the user's files stored in the Redis server.
int hdb_file_count(hdb_connection* con, const char* username) {
  return redis_hlen(con, username);
}

// Return a Boolean value indicating whether or not the user exists in
// the Redis server (i.e. whether or not he/she has files stored).
bool hdb_user_exists(hdb_connection* con, const char* username) {
  return redis_hexists(con, "usernames", username);
}

// Return a Boolean value indicating whether or not the file exists in
// the Redis server.
bool hdb_file_exists(hdb_connection* con, const char* username,
  const char* filename) {
  return redis_hexists(con, username, filename);
}

// Return a linked list of all of the specified user's file records stored
// in the Hooli database
hdb_record* hdb_user_files(hdb_connection* con, const char* username) {
  redisReply *reply = redis_hgetall(con, username);
  if(NULL == reply) {
    return NULL;
  }

  //Copy the username, as a constant will not work here
  char *usernameCpy = getStrCopyOfConst(username);

  //Set up the first(head) record in the linked list
  hdb_record *head = calloc(1, sizeof(hdb_record));
  hdb_record *current; //Points to current node as we traverse the linked list
  head->username = usernameCpy;

  //Copy contents to head node, since reply will be freed
  head->filename = getStrCopy(reply->element[0]->str);
  head->checksum = getStrCopy(reply->element[1]->str);
  head->next = NULL;
  current = head;

  //Keep adding records to the list until we're through them all
  for(int i = 2; i < reply->elements; i+=2) {
    //Create a new node
    hdb_record *temp = malloc(sizeof(hdb_record));
    //Copy contents to new node
    temp->username = usernameCpy;
    temp->filename = getStrCopy(reply->element[i]->str);
    temp->checksum = getStrCopy(reply->element[i+1]->str);
    temp->next = NULL;
    //Point current node's next to our new node
    current->next = temp;
    current = temp;
  }

  freeReplyObject(reply);
  return head;
}

// Free up the memory in a linked list allocated by hdb_user_files().
void hdb_free_result(hdb_record* record) {
  //Points to current node as we traverse the linked list
  hdb_record *current = record;
  hdb_record *next; //Will point to current node's next node

  free(current->username); //Because they all point to one shared username
  //Free up node's pointers and then node itself until we reach end of list
  while(current) {
    next = current->next;
    free(current->filename);
    free(current->checksum);
    free(current);
    current = next;
  }
}

// Delete the user and all of his/her file records from the Hooli database.
int hdb_delete_user(hdb_connection* con, const char* username) {
  int del = redis_del(con, username);
  int hdel = redis_hdel(con, "usernames", username);
  return del + hdel;
}

// Authenticate a user by password and return a generated token
char* hdb_authenticate(hdb_connection* con, const char* username,
  const char* password) {
  // NOTE: Passwords are stored under a hash called 'passwords', under the
  // field of the appropriate username ('passwords' username password)
  // NOTE: Tokens are stored as ('tokens' token username)
  
  // Non-constant copies of username and password
  char* usernameCpy = getStrCopyOfConst(username);
  char* passwordCpy = getStrCopyOfConst(password);

  // If the user exists, verify the specified password
  if(hdb_user_exists(con, usernameCpy)) {
    char* stored_pass = redis_hget(con, "usernames", usernameCpy);
    if(0 != strcmp(password, stored_pass)) {
      free(usernameCpy);
      free(passwordCpy);
      free(stored_pass);
      return NULL;
    }
    free(stored_pass);
  } else {
    redis_hset(con, "usernames", usernameCpy, passwordCpy);
  }
  // Create a 16-character token for user validation
  char* token = generateToken(16);

  // Store the token in redis and return it
  redis_hset(con, "tokens", token, usernameCpy);
  free(usernameCpy);
  free(passwordCpy);
  return token;
}

// Verify a specified token, returning the associated username if valid
// or NULL otherwise
char* hdb_verify_token(hdb_connection* con, const char* token) {
  if(redis_hexists(con, "tokens", token)) {
    return redis_hget(con, "tokens", token);
  } else {
    return NULL;
  }
}