# Setting up compiler and stuff
CC = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
all: a3w23


a3s: a3w23
	./$< -s 9722

a3c1: a3w23
	./$< -c 1 a3-cus1.dat 127.0.0.1 9722

a3c2: a3w23
	./$< -c 2 a3-cus1.dat localhost 9722

# I like this girl, but I'm afraid she doesn't like me back.
a3w23: a3w23.o
	$(CC) -pthread -o $@ $<

a3w23.o: a3w23.cpp
	$(CC) $(C_FLAGS) -c $<


fmt.man:
	man fmt | fmt -60 > $@

# Making a tarball
tar: clean
	tar -czvf wongwandanee-a3.tar.gz a3w23.cpp Makefile a3-ex1.dat README.md README.pdf

# For cleaning
.PHONY: clean
clean:
	rm -f *.o fmt.man a3w23 *.tar.gz