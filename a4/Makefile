# Setting up compiler and stuff
CC = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
all: a4w23


# I like this girl, but I'm afraid she doesn't like me back.
a4w23: a4w23.o
	$(CC) -pthread -o $@ $<

a4w23.o: a4w23.cpp
	$(CC) $(C_FLAGS) -c $<


# Making a tarball
tar: clean
	tar -czvf wongwandanee-a4.tar.gz a4w23.cpp Makefile README.md README.pdf

# For cleaning
.PHONY: clean
clean:
	rm -f *.o a4w23 *.tar.gz