# Setting up compiler and stuff
CC = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
all: a4w23

# Testing target
test:	a4w23
	./$< a4-cus1.dat 1000 50

# This is our benchmarking test case
test2:	a4w23
	./$< a4-cus2.dat 500 1000

test3:	a4w23
	./$< a4-cus3.dat 500	40

test4:	a4w23
	./$< a4-ex1.dat 100	20

# This creates the main executable. It's linking pthreads
a4w23: a4w23.o
	$(CC) -pthread -o $@ $<

a4w23.o: a4w23.cpp
	$(CC) $(C_FLAGS) -c $<


# Making a tarball
tar: clean
	tar -czvf wongwandanee-a4.tar.gz a4w23.* Makefile README.* a4-cus* a4-ex*

untar:
	rm -f *.tar.gz

# For cleaning
.PHONY: clean
clean:
	rm -f *.o a4w23