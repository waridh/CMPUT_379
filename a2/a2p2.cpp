#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define 	NCLIENT = 3;

//=============================================================================
// Structs

typedef struct  {
	// Struct for the send thread
	char * inFile;
	char * c2s_fifo;
} csend_t;

//=============================================================================
// Error handling
void cmd_arg_err()  {
	// Function deals with telling the user to follow the input requirement
	std::cout << std::endl;
	std::cout << "Invalid command line arguments, please use these inputs:"
	<< std::endl << std::endl << "a2p2 -s" << std::endl
	<< "a2p2 -c idNumber inputFile" << std::endl << std::endl;
	exit(EXIT_FAILURE);
}

void thread_create_fail()  {
	// For when you can't make a thread
	std::cout << "Failed to create thread" << std::endl;
	exit(EXIT_FAILURE);
}

//=============================================================================
// Utilities



//=============================================================================
// Submain/loops

void * client_cmd_send(void * arg)  {
	/* This function uses the io stream to send cmd lines to the server*/
	csend_t *				csend = (csend_t *) arg;

	int 						fd;
	std::fstream		fp(csend->inFile);
	std::string 		line;
	std::cout << "Got into thread" << std::endl;	
	std::cout << csend->inFile << std::endl;

	//fd = open(c2s_fifo, O_WRONLY);

	while (std::getline(fp, line))  {
		// Loop for grabbing cmd lines from the client file
		if ((line[0] == '#') || (line[0] == '\n') || (line.size() == 0))  {
			// Skipping if it's a comment, or line is empty
			continue;
		}
		std::cout << line << std::endl;
	}
	//close(fd);
}

//=============================================================================
// Main functions
void server_main()  {
	// This function will serve as the main function for the server

	// Initilization
	int					idNumber = 0;
	std::cout << "Running the server" << std::endl;
	return;
};

void client_main(int idNumber, char * inputFile)  {
	// Variable initialization
	char 						c2s_fifo[10];
	csend_t					csend;
	int							err;
	pthread_t				tids;
	std::fstream	fp(inputFile);
	// This function will serve as the main function for the client
	std::cout << "Running the client" << std::endl;

	// Need to create the fifo name
	sprintf(c2s_fifo, "fifo-0-%d", idNumber);
	csend.c2s_fifo = c2s_fifo;
	std::cout << "Making pipe now" << std::endl;
	// Making the fifo file
	mkfifo(csend.c2s_fifo, 0666);
	csend.inFile = inputFile;
	// Creating thread for sending data to the server
	std::cout << "About to run the thread" << std::endl;
	if (pthread_create(
		&tids,
		NULL,
		client_cmd_send,
		(void *) &csend
	) != 0)  {
		// Thread creation failure handling
		thread_create_fail();
	};

	pthread_join(tids, NULL);

	// client_cmd_send(&fp, c2s_fifo);

	return;
}

int main(int argc, char * argv[])  {
	// Initialization
	
	// Command line check
	if ((argc != 2) && (argc != 4))  {
		cmd_arg_err();
	};

	if ((argc == 2) && (strcmp(argv[1], "-s") == 0))  {
		// Check for server activation
		server_main();
	}  else if ((argc == 4) && (strcmp(argv[1], "-c") == 0))  {
		// Check for client
		client_main(strtol(argv[2], NULL, 10), argv[3]);
	}  else  {
		cmd_arg_err();
	}
	// Else we are able to run the two main programs
	return 0;
}