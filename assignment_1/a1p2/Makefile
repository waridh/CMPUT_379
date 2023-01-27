C = g++
C_FLAGS = -std=c++11 -g -Wall

# The top command is to compile the program and assign an alias for it
compile: a1p2
	alias a1p2="./a1p2"

# Simply turning the object into an executable
a1p2: a1p2.o
	$(CC) -o $@ $<

# Creating the C++ object
a1p2.o: a1p2.cpp
	$(CC) $(C_FLAGS) -c $<

# For cleaning
clean:
	rm -f a1p2 out1 *.o
