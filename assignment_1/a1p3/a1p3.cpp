#include <bits/stdc++.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sys/times.h>
 #include <sys/wait.h>
#include <unistd.h>

#define MAXLINE 128		// Max # of characters in an input line
#define MAX_NTOKEN 16	// Max # of tokens in any input line
#define MAXWORD 20		// Max # of characters in any token
#define NPROC 5				// Max # of commands in a test file

typedef struct	{
	/*	This is basically the same struct as from the first part where we were
	collecting those tokens.
	*/
	char token[MAX_NTOKEN + 1][MAXWORD + 1];
}	tokens;

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
};

//=============================================================================
// This section is for putting the string inputs from the stdin into the exec

unsigned int gettokens(std::string &cmdline, tokens *toks)  {
	/*This function will get the tokens and get an array of cstrings back*/
	// char **tokener = new char*[MAX_NTOKEN+1];
	char token3[MAXWORD];
	std::string token4[MAX_NTOKEN + 1];
	std::cout << MAX_NTOKEN + 1 << std::endl;

	char * cmdline2 = new char[cmdline.length() + 1];
	strcpy(cmdline2, cmdline.c_str());
	// Getting the whitespaces
	char WSPACE[] = " \t"; char *buffet = strtok(cmdline2, WSPACE);
	unsigned int count = 0;

	while (buffet != NULL)  {
		std::cout << count << std::endl;
		std::cout << buffet << std::endl;
		// Using strcpy to move the token to the struct
		// tokener[count] = new char[MAXWORD + 1];
		std::cout << "allocated" << std::endl;
		strcpy(token3, buffet);
		strcpy(toks -> token[count], token3);
		// strcpy(tokener[count], buffet);
		token4[count] = buffet;

		count++;
		std::cout << toks -> token[count] << std::endl;
		// std::cout << *(tokener + count) << std::endl;
		std::cout << token3 << std::endl;
		std::cout << token4[count] << std::endl;
		buffet = strtok(NULL, WSPACE);
	};
	delete[] cmdline2;
	return count;
};

void runline(std::string &cmdline, pid_t &pid)  {
	/*
	This function will run the cmdline given by the input
	*/
	//TODO This stupid function
	pid = fork();
	if (pid < 0)  {
		std::cout << "Forking failed" << std::endl;
		std::cout << "Exiting program..." << std::endl;
		exit(EXIT_FAILURE);
	}  else if (pid == 0)  {
		// This is the child process
		tokens toks;
		memset(&toks, 0, sizeof(toks));

		
		char cmdline2 = new char[cmdline.length() + 1];
		strcpy(cmdline2, cmdline.c_str());
		char WSPACE[] = "\t ", *buffer = strtok(cmdline2, WSPACE);
		unsigned int count = 0;

		// Debugging
		std::cout << "Debugging the tokens struct" << std::endl;
		std::cout << count << std::endl;
		for (unsigned int i = 0; i < count; i++)  {
			std::cout << toks.token[count] << std::endl;
		}

		execlp("timeout", "timeout", "3", "./myclock", "outA", (char*) NULL);
		exit(EXIT_SUCCESS);
	}  else  {
		// This the is parent process. PID is the child id
		wait(NULL);
	};
	return;
};

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
	pid_t pid; std::string test = "timeout 3 ./myclock outA";

	// The initial time
	time1 = times(&cpu1);

	// Initializing for input and then take it. Intermediate variable
	std::string intermediateStr[NPROC + 1]; collectinputs(intermediateStr);

//=============================================================================
	// Trying to execute the programs in the inputs
	
	runline(test, pid);

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

