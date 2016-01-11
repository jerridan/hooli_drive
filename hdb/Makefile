CC      = gcc
CFLAGS  = -g -O0 -Wall -Werror -std=gnu11 -I../common
LDFLAGS = -L.
VPATH   = ../common

.PHONY : all test clean

all: libhdb.a testlibhdb

libhdb.a: hdb.o redis_queries.o string_functions.o
	ar rcs libhdb.a hdb.o redis_queries.o string_functions.o

hdb.o: hdb.c hdb.h
	$(CC) $(CFLAGS) -o $@ -c $<

redis_queries.o: redis_queries.c redis_queries.h
	$(CC) $(CFLAGS) -o $@ -c $<

string_functions.o: string_functions.c string_functions.h
	$(CC) $(CFLAGS) -o $@ -c $<

testlibhdb: libhdb.a testlibhdb.o
	$(CC) -o $@ $^ $(LDFLAGS) -lhdb -lcheck -lhiredis -lm -lpthread -lrt

testlibhdb.o: testlibhdb.c hdb.h
	$(CC) $(CFLAGS) -o $@ -c $<

test: testlibhdb
	./testlibhdb

clean:
	rm -f *.o *.a testlibhdb
