C = g++
C_FLAGS = -std=c++11 -g -Wall
BASHRC_PATH = ~/.bashrc

# The top command is to compile the program. It won't really run anything
all: a1p2

# Run this command after the program has already been compiled to use the w
# flag
a1p2w: a1p2
	./a1p2 w

# This target runs the s flag on part 2
a1p2s: a1p2
	./a1p2 s

# Simply turning the object into an executable
a1p2: a1p2.o
	$(CC) -o $@ $<

# Creating the C++ object
a1p2.o: a1p2.cpp
	$(CC) $(C_FLAGS) -c $<

# For cleaning
.PHONY: clean
clean:
	rm -f a1p2 out1 *.o

