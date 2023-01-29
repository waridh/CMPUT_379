CC = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
all: a1p3 

# Quick running the program
a1p30: a1p3
	./a1p3 0

a1p31: a1p3
	./a1p3 1

a1p3neg: a1p3
	./a1p3 -1

# Simply turning the object into an executable
a1p3: a1p3.o
	$(CC) -o $@ $<

# Creating the C++ object
a1p3.o: a1p3.cpp
	$(CC) $(C_FLAGS) -c $<

# For cleaning
.PHONY: clean
clean:
	rm -f a1p3 *.o

