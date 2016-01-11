/*****************************************************************************
 * redis_queries.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Functions for making Redis queries to a server
 ****************************************************************************/
 
#ifndef REDIS_QUERIES_H
#define REDIS_QUERIES_H

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

#include "string_functions.h"

 // Connection to the Hooli database
typedef void* hdb_connection;

//Performs an HSET query to a Redis context
//Returns 0 if new field was set, 1 if field existed and was updated
int redis_hset(hdb_connection *con, char *hashkey, char *field, char *value);

//Performs an HDEL query to a Redis context
//Returns number of fields deleted from the hashkey
int redis_hdel(hdb_connection *con, const char *hashkey, const char *field);

//Performs an HGET query to a Redis context
//Return NULL if field or hashkey do not exist, else return field value
char *redis_hget(hdb_connection *con, const char *hashkey, const char *field);

//Performs an HLEN query to a Redis context
//Returns number of field present for hashkey
int redis_hlen(hdb_connection *con, const char *hashkey);

//Performs an EXISTS query to a Redis context
//Returns a Boolean value indicating whether hashkey exists
bool redis_exists(hdb_connection *con, const char *hashkey);

//Performs an HEXISTS query to a Redis context
//Returns a Boolean value indicating whether field exists
bool redis_hexists(hdb_connection *con, const char *hashkey,
  const char *field);

//Performs an HGETALL query to a Redis context
//Returns NULL if no elements found for hashkey, else returns full redisReply
redisReply *redis_hgetall(hdb_connection *con, const char *hashkey);

//Performs an DEL query to a Redis context
//Returns number of keys deleted (0 or 1 in this case)
int redis_del(hdb_connection *con, const char *hashkey);

#endif