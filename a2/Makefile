CC = g++
#CC2 = gcc
C_FLAGS = -std=c++11 -g -Wall
#C_FLAGS2 = -std=c99 -Wall

# The top command is to compile the program and assign an alias for it
all: a2p1 fmt.man a2p2


# Testing a2p1
a2p1test: a2p1 fmt.man
	./a2p1 5 fmt.man 3500

# Compiles the part 1 program
a2p1: a2p1.o
	$(CC) -pthread -o $@ $<

a2p1.o: a2p1.cpp
	$(CC) $(C_FLAGS) -c $<

a2p2c1: a2p2
	./a2p2 -c 1 a2p2-ex1.dat

a2p2c2: a2p2
	./a2p2 -c 2 a2p2-ex1.dat

a2p2s:	a2p2
	./a2p2 -s

a2p2: a2p2.o
	$(CC) -pthread -o $@ $<

a2p2.o: a2p2.cpp
	$(CC) $(C_FLAGS) -c $<

fmt.man:
	man fmt | fmt -60 > $@

# Making a tarball
tar: clean
	tar -czvf wongwandanee-a2.tar.gz a2p1.cpp a2p2.cpp Makefile wongwandanee-a2-typeset.pdf a2p2-ex1.dat a2p2.data

# For cleaning
.PHONY: clean
clean:
	rm -f *.o fmt.man a2p1 a2p2 fifo* *.tar.gz