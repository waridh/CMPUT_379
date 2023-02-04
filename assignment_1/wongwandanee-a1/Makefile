CC = g++
#CC2 = gcc
C_FLAGS = -std=c++11 -g -Wall
#C_FLAGS2 = -std=c99 -Wall

# The top command is to compile the program and assign an alias for it
all: a1p3 do_work bash.man a1p2

# Running assignment 1 part 2
a1p2w: a1p2
	./a1p2 w

a1p2s: a1p2
	./a1p2 s

# Quick running the program
a1p30: a1p3 do_work bash.man
	./a1p3 0 < a1p3.data2

a1p31: a1p3 do_work bash.man
	./a1p3 1 < a1p3.data2

a1p3neg: a1p3 do_work bash.man
	./a1p3 -1 < a1p3.data2

# Making the do_work executable for testing
do_work: do_work.o
	$(CC) -o $@ $<

do_work.o: do_work.c
	$(CC) $(C_FLAGS) -c $<

# Making the bash.man file for testing also
bash.man:
	man bash > bash.man
# Simply turning the object into an executable
a1p3: a1p3.o bash.man do_work
	$(CC) -o $@ $<

# Creating the C++ object
a1p3.o: a1p3.cpp
	$(CC) $(C_FLAGS) -c $<

# Turning the a1p2 code into an executable
a1p2: a1p2.o
	$(CC) -o $@ $<

# a1p2 object
a1p2.o: a1p2.cpp
	$(CC) $(C_FLAGS) -c $<

# Making a tarball
tar: clean
	tar -czvf wongwandanee-a1.tar.gz *

# For cleaning
.PHONY: clean
clean:
	rm -f a1p2 a1p3 do_work *.o out* bash.man wongwandanee-a1.tar.gz

