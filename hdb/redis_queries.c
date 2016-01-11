/*****************************************************************************
 * redis_queries.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * Functions for making Redis queries to a server
 ****************************************************************************/

#include "redis_queries.h"

//Performs an HSET query to a Redis context
//Returns 0 if new field was set, 1 if field existed and was updated
int redis_hset(hdb_connection *con, char *hashkey, char *field, char *value) {
  redisReply *reply; //Declare redisReply object
  //Call HSET command
  reply = redisCommand((redisContext*)con, "HSET %s %s %s", hashkey, field, 
    value);
  //Copy reply value, and free reply
  int replyValue = reply->integer;
  freeReplyObject(reply);
  return replyValue;
}

//Performs an HDEL query to a Redis context
//Returns number of fields deleted from the hashkey
int redis_hdel(hdb_connection *con, const char *hashkey, const char *field) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "HDEL %s %s", hashkey, field);
  //Copy reply value, and free reply
  int numFieldsRemoved = reply->integer;
  freeReplyObject(reply);
  return numFieldsRemoved;
}

//Performs an HGET query to a Redis context
//Return NULL if field or hashkey do not exist, else return field value
char *redis_hget(hdb_connection *con, const char *hashkey,
  const char *field) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "HGET %s %s", hashkey, field);
  if(reply->type != REDIS_REPLY_STRING) {
    freeReplyObject(reply);
    return NULL;
  }
  //Copy reply value and free reply
  char *value = getStrCopy(reply->str);
  freeReplyObject(reply);
  return value;
}

//Performs an HLEN query to a Redis context
//Returns number of field present for hashkey
int redis_hlen(hdb_connection *con, const char *hashkey) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "HLEN %s", hashkey);
  //Copy reply value, and free reply
  int numFields = reply->integer;
  freeReplyObject(reply);
  return numFields;
}

//Performs an EXISTS query to a Redis context
//Returns a Boolean value indicating whether hashkey exists
bool redis_exists(hdb_connection *con, const char *hashkey) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "EXISTS %s", hashkey);
  //reply value will be 1 if hashkey exists, 0 otherwise - cast to boolean
  bool exists = (bool)(reply->integer);
  freeReplyObject(reply);
  return exists;
}

//Performs an HEXISTS query to a Redis context
//Returns a Boolean value indicating whether field exists
bool redis_hexists(hdb_connection *con, const char *hashkey,
  const char *field) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "HEXISTS %s %s", hashkey, field);
  //reply value will be 1 if field exists, 0 otherwise - cast to boolean
  bool exists = (bool)(reply->integer);
  freeReplyObject(reply);
  return exists;
}

//Performs an HGETALL query to a Redis context
//Returns NULL if no elements found for hashkey, else returns full redisReply
redisReply *redis_hgetall(hdb_connection *con, const char *hashkey) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "HGETALL %s", hashkey);
  //If nothing found at haskhey, return NULL
  if(!reply->elements) {
    freeReplyObject(reply);
    return NULL;
  }
  //libhdb makes greater use of redisReply object in this case, so return it
  return reply;
}

//Performs an DEL query to a Redis context
//Returns number of keys deleted (0 or 1 in this case)
int redis_del(hdb_connection *con, const char *hashkey) {
  redisReply *reply; //Declare redisReply object
  reply = redisCommand((redisContext*)con, "DEL %s", hashkey);
  //Copy reply value, and free reply
  int numKeysDeleted = reply->integer;
  freeReplyObject(reply);
  return numKeysDeleted;
}