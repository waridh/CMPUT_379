#include <iostream>
#include <sys/times.h>
#include <string>
#include <unistd.h>
#include <bits/stdc++.h>

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

void getlines()  {
	/*
	This function gets a line from the standard input
	*/
	std::string buffers;
	while(getline(std::cin, buffers, '\n'))  {
		if ( (buffers[0] == '#') || (buffers.size() == 0) )  {
			continue;
		}
		std::cout << buffers << std::endl;
	};
	
	return;
};

void collectinputs()  {
	/*
	This function collects all the lines of the standard inputs
	*/
	getlines();
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

	// Initializing for input
	std::string buffer;

	collectinputs();

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

