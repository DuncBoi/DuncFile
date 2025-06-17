#
# Makefile for lab 5 part 2
#

CC  = g++
CXX = g++

CFLAGS   = -g -Wall
CXXFLAGS = -g -Wall -std=c++17 -I ../part1

executable = mymake
headers = maker.h exception.h

$(executable): maker.o

# header dependency

mymake.o: $(headers)

maker.o: $(headers) helper.h

.PHONY: clean
clean:
	rm -f *.o *~ a.out core $(executable)

.PHONY: all
all: clean mymake
