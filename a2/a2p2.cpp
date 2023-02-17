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
	char * 				inFile;
	char * 				c2s_fifo;
	int						cid;
} csend_t;

typedef struct  {
	// For writing to fifo
	char * 				fifo_n;
	std::string		line;
} wfifo_t;

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

void fifo_open_fail()  {
	std::cout << "Failed to open the fifo" << std::endl;
	exit(EXIT_FAILURE);
}

//=============================================================================
// Utilities
void * fifo_write(void * arg)  {
	/* This function uses threads to write to fifo*/
	wfifo_t *					wfifo = (wfifo_t *) arg;
	int								fd;
	fd = open(wfifo->fifo_n, O_WRONLY | O_NONBLOCK);
	write(fd, wfifo->line.c_str(), strlen(wfifo->line.c_str()) + 1);
	close(fd);
	pthread_exit(NULL);
}


//=============================================================================
// Submain/loops

void * client_cmd_send(void * arg)  {
	/* This function uses the io stream to send cmd lines to the server*/
	csend_t *				csend = (csend_t *) arg;
	int 						fd;
	std::fstream		fp(csend->inFile);
	std::string 		line;
	wfifo_t					wfifo;
	fd = open(csend->c2s_fifo, O_WRONLY | O_NONBLOCK);

	wfifo.fifo_n = csend->c2s_fifo;
	while (std::getline(fp, line))  {
		// Loop for grabbing cmd lines from the client file
		if ((line[0] == '#') || (line[0] == '\n') || (line.size() == 0))  {
			// Skipping if it's a comment, or line is empty
			continue;
		}
		// Debugging	
		std::cout << line << std::endl;
		wfifo.line = line;
		// line_s = line.c_str();
		// Writing the line to fifo
		write(fd, line.c_str(), strlen(line.c_str()) + 1);
		std::cout << "SENT TO PIPE" << std::endl;
	}
	close(fd);
	sleep(1);
	pthread_exit(NULL);
}

void * client_reciever(void * arg)  {
	/* This function will be used with a thread to take inputs*/
	csend_t *					crecieve = (csend_t *) arg;
	int								fd;

	fd = open(crecieve->c2s_fifo, O_RDONLY);
}

void * server_reciever(void * arg)  {
	/* This thread function will loop and read inputs */
	while (1) {
		// Wait for a signal

	}
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
	char						s2c_fifo[10];
	csend_t					csend;
	csend_t					crecieve;
	int							err;
	pthread_t				tids;
	pthread_t				tidr;
	std::fstream		fp(inputFile);
	// This function will serve as the main function for the client
	std::cout << "main: do_client (idNumber = " << idNumber
	<< ", inputFile = " << inputFile << ")" << std::endl;

	// Need to create the fifo name
	sprintf(c2s_fifo, "fifo-0-%d", idNumber);
	sprintf(s2c_fifo, "fifo-%d-0", idNumber);
	csend.c2s_fifo = c2s_fifo;
	crecieve.c2s_fifo = s2c_fifo;
	// Making the fifo file
	mkfifo(csend.c2s_fifo, 0666);
	mkfifo(crecieve.c2s_fifo, 0666);
	// Fully initialize the struct being sent to the thread
	csend.inFile = inputFile;
	csend.cid = idNumber;
	// Creating thread for sending data to the server
	std::cout << "CREATEING THREAD" << std::endl;
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