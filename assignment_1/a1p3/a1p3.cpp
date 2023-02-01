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

unsigned int getlines(std::string buffers[])  {
	/*
	This function gets a line from the standard input
	*/
	unsigned int linecount = 0;
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
	
	return linecount;
};

unsigned int collectinputs(std::string rawlines[])  {
	/*
	This function collects all the lines of the standard inputs
	*/
	// Allocated an extra room so that getline() doesn't segmentation fault
	unsigned int lines = getlines(rawlines);

	return lines;
};

//=============================================================================
// This section is for putting the string inputs from the stdin into the exec

unsigned int gettokens(std::string &cmdline, tokens &tok)  {
	// reset mem
	memset(&tok, 0, sizeof(tok));
	// Sample string for testing
	char *inStr = new char[cmdline.length() + 1];

	strcpy(inStr, cmdline.c_str());
  // Initialize the count
  unsigned int count = 0;
  // The delimiters
  char WSPACE[] = "\n \t";

  // Creating the output in sigular token.
  char *buffet = strtok(inStr, WSPACE);

  while (buffet != NULL)  {
		// Using strcpy to move the token into the struct
		strcpy(tok.token[count], buffet);	
		// Adding to the count
		count++;
		// Obtaining the next token
    buffet = strtok(NULL, WSPACE);
	};	
	
	return count;
};

void runline(std::string &cmdline)  {
	/*
	This function will run the cmdline given by the input
	*/
	//TODO This stupid function
	pid_t pid = fork();
	if (pid < 0)  {
		std::cout << "Forking failed" << std::endl;
		std::cout << "Exiting program..." << std::endl;
		exit(EXIT_FAILURE);
	}  else if (pid == 0)  {
		// This is the child process
		tokens toks;
		 unsigned int count = gettokens(cmdline, toks);
		 // We need to convert the type to something that execvp can read
		 char * args[MAX_NTOKEN + 1];
		 for (int i = 0; i < MAX_NTOKEN + 1; i++)  {
			 // Transfer the tokens from the struct to this one
			 args[i] = toks.token[i];
		 };
		 args[MAX_NTOKEN + 1] = NULL;
		
		// Now we need to input the characters into execlp
		char * arglist[] = {"timeout", "3", "./myclock", "outA", NULL};
		execvp(args[0], args);
		exit(EXIT_SUCCESS);
	}  else  {
		// This the is parent process. PID is the child id
		wait(NULL);
	};
	return;
};

void childcontroller(std::string *fullinput, unsigned int &lines)  {
	// This function will send the command from input to child processes.
	std::cout << fullinput[0] << std::endl;
	std::cout << lines << std::endl;
	for (unsigned int i = 0; i < lines; i++)  {
		// Running the lines
		printf("Running line %i\n=============================================\n", i);
		runline(fullinput[i]);
	}
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
	std::string test = "timeout 3 ./myclock outA";

	// The initial time
	time1 = times(&cpu1);

	// Initializing for input and then take it. Intermediate variable
	std::string intermediateStr[NPROC + 1];
	unsigned int lines = collectinputs(intermediateStr);


//=============================================================================
	// Trying to execute the programs in the inputs
	childcontroller(intermediateStr, lines);

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

