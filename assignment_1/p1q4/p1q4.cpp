// Assignment 1 Part 1 Question 4

/*
	When encountering an error, Many UNIX system calls return -1 and set the
	integer errno to a value indicating the reason of the error. Give a C
	code fragment to print on the stdout (rather than stderr) an error
	message string corresponding to the errno value.
*/

#include <cerrno>
#include <iostream>
#include <cstdio>
#include <cstring>

int main(int argc, char *argv[])  {
	errno = 3;
	printf("Error: %s\n", strerror(errno));
	exit(0);
}

