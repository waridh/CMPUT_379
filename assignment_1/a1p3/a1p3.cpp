#include <bits/stdc++.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sys/times.h>
#include <string>
#include <unistd.h>

#define MAXLINE 128		// Max # of characters in an input line
#define MAX_NTOKEN 16	// Max # of tokens in any input line
#define MAXWORD 20		// Max # of characters in any token
#define NPROC 5				// Max # of commands in a test file

typedef struct	{
	/*	This is basically the same struct as from the first part where we were
	collecting those tokens.
	*/
	char token[MAXLINE/2][MAXWORD];
}	token;

void	notenoughargs()  {
	// Functionalized the error
	std::cout << "There are not enough command-line arguments." << std::endl;
	std::cout << "Please run the program again using proper formats." << std::endl;
	std::cout << "Quitting..." << std::endl;
	exit(EXIT_SUCCESS);
};

void getlines(std::string buffers[])  {
	/*
	This function gets a line from the standard input
	*/
	int linecount = 0;
	std::string buffer;

	// Simply get all the line from the input stream
	while (getline(std::cin, buffers[linecount], '\n'))  {
		// We do not want to read comments and empty lines
		if ( (buffers[linecount][0] == '#') || (buffers[linecount].size() == 0) )  {
			continue;
		}

		// Update the index
		linecount++;
	};
	std::cout << "We have taken all the inputs" << std::endl;
	
	return;
};

void collectinputs(std::string rawlines[])  {
	/*
	This function collects all the lines of the standard inputs
	*/
	// Allocated an extra room so that getline() doesn't segmentation fault
	getlines(rawlines);

	for (int i = 0; i < 5; i++)  {
		std::cout << rawlines[i] << std::endl;
	}
	return;
}

void runline(string cmdline, pid_t pid)  {
	/*
	This function will run the cmdline given by the input
	*/

	return;
}

int main(int argc, char *argv[])  {
	if (argc < 2)  {
		// Error handling
		notenoughargs();
	};

//=============================================================================
	// Declaring for times(). Both the starting and ending times.
	long clktck=sysconf(_SC_CLK_TCK);
	clock_t time1, time2;
	struct tms cpu1; struct tms cpu2;

	// The initial time
	time1 = times(&cpu1);

	// Initializing for input and then take it. Intermediate variable
	std::string intermediateStr[NPROC + 1]; collectinputs(intermediateStr);

//=============================================================================
	// Trying to execute the programs in the inputs



//=============================================================================

	if (argv[1][0] == '0' && (strlen(argv[1]) == 1))  {
		std::cout << "The argument was 0" << std::endl;
		return 0;
	}  else if ((argv[1][0] == '1') && (strlen(argv[1]) == 1))  {
		std::cout << "was 1" << std::endl;
		return 0;
	}  else if ((argv[1][0] == '-') && (argv[1][1] == '1') && (strlen(argv[1]) == 2))  {
		std::cout << "neg 1" << std::endl;
		return 0;
	}  else  {
		// More error handling
		std::cout << "Command-line argument not detected" << std::endl;
	}
	return 0;
}

