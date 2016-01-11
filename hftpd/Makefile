CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11 -I../common -I../hdb
LDFLAGS = -L. -L../hdb
LIBS    = -lz -lhdb -lhiredis
VPATH   = ../common ../hdb

.PHONY: all clean

all: libhdb.a hftpd

libhdb.a: hdb.o redis_queries.o string_functions.o
	ar rcs libhdb.a hdb.o redis_queries.o string_functions.o

hdb.o: hdb.c hdb.h
	$(CC) $(CFLAGS) -o $@ -c $<

hftpd: hftpd.o hftpd_arg_handler.o string_functions.o udp_server.o udp_sockets.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

hftpd.o: hftpd.c hftpd.h
	$(CC) $(CFLAGS) -o $@ -c $<

hftpd_arg_handler.o: hftpd_arg_handler.c hftpd_arg_handler.h
	$(CC) $(CFLAGS) -o $@ -c $<

string_functions.o: string_functions.c string_functions.h
	$(CC) $(CFLAGS) -o $@ -c $<

udp_server.o: udp_server.c udp_server.h
	$(CC) $(CFLAGS) -o $@ -c $<

udp_sockets.o: udp_sockets.c udp_sockets.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a hftpd