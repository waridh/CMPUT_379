CC = g++
#CC2 = gcc
C_FLAGS = -std=c++11 -g -Wall
#C_FLAGS2 = -std=c99 -Wall

# The top command is to compile the program and assign an alias for it
all: a1p3 do_work

# Quick running the program
a1p30: all
	./a1p3 0 < a1p3.data

a1p31: all
	./a1p3 1 < a1p3.data

a1p3neg: all
	./a1p3 -1 < a1p3.data

# Making the do_work executable for testing
do_work: do_work.o
	$(CC) -o $@ $<

do_work.o: do_work.c
	$(CC) $(C_FLAGS) -c $<
# Simply turning the object into an executable
a1p3: a1p3.o
	$(CC) -o $@ $<

# Creating the C++ object
a1p3.o: a1p3.cpp
	$(CC) $(C_FLAGS) -c $<

# For cleaning
.PHONY: clean
clean:
	rm -f a1p3 do_work *.o

