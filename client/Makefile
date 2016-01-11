CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11 -I../common
LDFLAGS = -L.
LIBS    = -lz
VPATH   = ../common

.PHONY: all clean

all: client

client: client.o dir_functions.o argument_handler.o tcp_client.o \
	hdmp_protocols.o string_functions.o hmds_liason.o hftp_liason.o \
	udp_sockets.o udp_client.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

client.o: client.c client.h
	$(CC) $(CFLAGS) -o $@ -c $<

dir_functions.o: dir_functions.c dir_functions.h
	$(CC) $(CFLAGS) -o $@ -c $<

argument_handler.o: argument_handler.c argument_handler.h
	$(CC) $(CFLAGS) -o $@ -c $<

tcp_client.o: tcp_client.c tcp_client.h
	$(CC) $(CFLAGS) -o $@ -c $<

hdmp_protocols.o: hdmp_protocols.c hdmp_protocols.h
	$(CC) $(CFLAGS) -o $@ -c $<

string_functions.o: string_functions.c string_functions.h
	$(CC) $(CFLAGS) -o $@ -c $<

hmds_liason.o: hmds_liason.c hmds_liason.h
	$(CC) $(CFLAGS) -o $@ -c $<

hftp_liason.o: hftp_liason.c hftp_liason.h
	$(CC) $(CFLAGS) -o $@ -c $<

udp_sockets.o: udp_sockets.c udp_sockets.h
	$(CC) $(CFLAGS) -o $@ -c $<

udp_client.o: udp_client.c udp_client.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o *.a client
