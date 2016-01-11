CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11 -I../common -I../hdb
LDFLAGS = -L. -L../hdb
LIBS    = -lhdb -lhiredis
VPATH   = ../common ../hdb

.PHONY: all clean

all: libhdb.a hmds

libhdb.a: hdb.o redis_queries.o string_functions.o
	ar rcs libhdb.a hdb.o redis_queries.o string_functions.o

hdb.o: hdb.c hdb.h
	$(CC) $(CFLAGS) -o $@ -c $<

redis_queries.o: redis_queries.c redis_queries.h
	$(CC) $(CFLAGS) -o $@ -c $<

hmds: hmds.o hmds_arg_handler.o tcp_server.o string_functions.o \
	hdmp_protocols.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

hmds.o: hmds.c hmds.h
	$(CC) $(CFLAGS) -o $@ -c $<

hmds_arg_handler.o: hmds_arg_handler.c hmds_arg_handler.h
	$(CC) $(CFLAGS) -o $@ -c $<

tcp_server.o: tcp_server.c tcp_server.h
	$(CC) $(CFLAGS) -o $@ -c $<

string_functions.o: string_functions.c string_functions.h
	$(CC) $(CFLAGS) -o $@ -c $<

hdmp_protocols.o: hdmp_protocols.c hdmp_protocols.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a hmds
