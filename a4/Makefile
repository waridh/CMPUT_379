# Setting up compiler and stuff
CC = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
all: a4w23

# Testing target
test:	a4w23
	./$< a4-cus1.dat 1000 1000

# This creates the main executable. It's linking pthreads
a4w23: a4w23.o
	$(CC) -pthread -o $@ $<

a4w23.o: a4w23.cpp
	$(CC) $(C_FLAGS) -c $<


# Making a tarball
tar: clean
	tar -czvf wongwandanee-a4.tar.gz a4w23.cpp a4w23.h Makefile README.md README.pdf

# For cleaning
.PHONY: clean
clean:
	rm -f *.o a4w23 *.tar.gz