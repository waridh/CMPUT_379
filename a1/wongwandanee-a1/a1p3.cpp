#include <bits/stdc++.h>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <map>
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

	std::cout << std::endl;
	std::cout << "Taking in command line inputs:" << std::endl;

	// Simply get all the line from the input stream
	while (getline(std::cin, buffers[linecount], '\n'))  {
		// We do not want to read comments and empty lines
		if ( (buffers[linecount][0] == '#') || (buffers[linecount].size() == 0) )  {
			continue;
		}
		std::cout << " print_cmd(): [" << buffers[linecount] << ']' << std::endl;
		// Update the index
		linecount++;
		if (linecount == NPROC)  {
			// We only want to take the lines specified in the rubric (5 lines)
			break;
		}
	};
	std::cout << std::endl;
	
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
	/* This function converts a command line string into 2d character array of
	tokens. */
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

void runline(std::string &cmdline, pid_t &pids, unsigned int &k)  {
	/*
	This function will run the cmdline given by the input
	*/
	
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
		 for (unsigned int i = 0; i < count; i++)  {
			 // Transfer the tokens from the struct to this one
			 args[i] = toks.token[i];
		 };
		 args[count] = NULL;
	
		execvp(args[0], args);
		
		// For failed exec outputs
		std::cout << "child (" << getpid()
		<< "): unable to execute '" << cmdline << "'" << std::endl << std::endl;
		abort();
		return;
	}  else  {
		// This the is parent process. PID is the child id. Sending it out
		std::cout << ' ' << k << " [" << pid << ": '" << cmdline << "']" << std::endl;
		pids = pid;
		
		return;
	};
	return;
};

void childcontroller(
	std::string *fullinput,
	unsigned int &lines,
	int argc,
	char *argv[]
	)  {
	// This function will send the command from input to child processes.
	std::cout <<"There are " << lines << " command lines"
	<< " to be executed" << std::endl;
	// Allocating memory to pids array + other initialization
	pid_t *pids = new pid_t[lines]; int status;
	pid_t pid; bool count = true;
	std::map<pid_t, std::string> cmdmap; 

	std::cout << std::endl;
	std::cout << "Process table:" << std::endl;
	for (unsigned int i = 0; i < lines; i++)  {
		// Running the lines
		runline(fullinput[i], pids[i], i);
		cmdmap[pids[i]] = fullinput[i];
	};
	std::cout << std::endl;

			
	if (argv[1][0] == '0' && (strlen(argv[1]) == 1))  {
		// This option makes us go straight to step 5
		std::cout << "Not wating for any child processes" << std::endl
		<< std::endl;
		
	}  else if ((argv[1][0] == '1') && (strlen(argv[1]) == 1))  {
		// We want to wait for exactly 1 child to return before continuing
		std::cout << "Waiting for one child process to complete" << std::endl
		<< std::endl;
		pid = waitpid(-1, &status, 0);
    std::cout << std::endl;
    std::cout << "process (" << pid << "): exited (status = " << status
    << ')' << std::endl;
	
	}  else if ((argv[1][0] == '-') && (argv[1][1] == '1')
			&& (strlen(argv[1]) == 2))  {
		// We want to wait for all programs to finish before continuing
		std::cout << "Waiting for all child processes to complete" << std::endl;
		// Need to cite the stack overflow code for this bit
		std::cout << std::endl;
		/* This while loop for waiting for all child process to run*/
		while ((pid = waitpid(-1, &status, 0)) != -1)  {
			// We are simply looping until all the processes are done
			
			// Not outputting this because it means that the program did not execute
      if ((status == 134) || (status == 6))  {
        continue;
      }
			if (count)  {
				// Print a newline to separate the output
				std::cout << std::endl;
				count = false;

			}
			std::cout << "process (" << pid << ": '" << cmdmap[pid]
			<< "'): exited (status = " << status << ')' << std::endl;
		};
	
	}  else  {
		// More error handling
		std::cout << "Command-line argument not detected" << std::endl;
		std::cout << "Please try again with proper arguments" << std::endl;
		exit(EXIT_SUCCESS);
	}

	
	return;
}

//=============================================================================
// Part 6 output print function
void timeprint(
	clock_t &time1,
	clock_t &time2,
	struct tms &cpu1,
	struct tms &cpu2)  {
	/* This function will try to print to format of figure 8.31. We are using the
testbook: Advanced Programming in the UNIX Environment third Edition*/
	static long clktck = 0;
	if (clktck == 0)  {  // If we don't already have the tick
		clktck=sysconf(_SC_CLK_TCK);
		if (clktck < 0)  {
			std::cout << "sysconf error" << std::endl;  // Error handling from apue
			exit(EXIT_FAILURE);
		}
	}
	// Most of the code is gathered from the APUE textbook here
  clock_t real = time2 - time1;
	std::cout << std::endl << "Recorded time:" << std::endl;
  printf(" real: %7.2f sec.\n", real / (double) clktck);
  printf(" user: %7.2f sec.\n",
  (cpu2.tms_utime - cpu1.tms_utime) / (double) clktck);
  printf(" sys: %7.2f sec.\n",
  (cpu2.tms_stime - cpu1.tms_stime) / (double) clktck);
  printf(" child user: %7.2f sec.\n",
  (cpu2.tms_cutime - cpu1.tms_cutime) / (double) clktck);
  printf(" child sys: %7.2f sec.\n",
  (cpu2.tms_cstime - cpu1.tms_cstime) / (double) clktck);
	return;
}

int main(int argc, char *argv[])  {
	if (argc < 2)  {
		// Error handling
		notenoughargs();
	};

//=============================================================================
	// Step 1 - The initial times call
	// Declaring for times(). Both the starting and ending times.
	clock_t time1, time2;
	struct tms cpu1; struct tms cpu2;
	// calling the first times()
	time1 = times(&cpu1);

//=============================================================================
	// Step 2 - Taking in command line inputs from stdin
	// Initializing for input and then take it. Intermediate variable
	std::string intermediateStr[NPROC + 1];
	unsigned int lines = collectinputs(intermediateStr);

//=============================================================================
	// Step 3 and 4 - Trying to execute the programs in the inputs and wait
	childcontroller(intermediateStr, lines, argc, argv);

//=============================================================================
	// Step 5 - Calling time again
	time2 = times(&cpu2);

//=============================================================================
	// Step 6 - Getting the output to match the figure
	timeprint(time1, time2, cpu1, cpu2);
		
	return 0;
}

