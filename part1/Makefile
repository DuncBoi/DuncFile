#
# Makefile for lab 5 part 1
#

CC  = g++
CXX = g++

CFLAGS   = -g -Wall
CXXFLAGS = -g -Wall -std=c++17

.PHONY: default
default: mdb-add mdb-cat

mdb-cat: mdb.o

mdb-add: mdb.o

# header dependency

mdb-add.o: mdb.h xdb.h

mdb-cat.o: mdb.h xdb.h

mdb.o: mdb.h xdb.h

.PHONY: clean
clean:
	rm -f *.o *~ a.out core mdb-add mdb-cat

.PHONY: all
all: clean default

