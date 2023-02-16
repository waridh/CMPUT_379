#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#define 	NCLIENT = 3;

//=============================================================================
// Error handling
void * cmd_arg_err()  {
	// Function deals with telling the user to follow the input requirement
	std::cout << std::endl;
	std::cout << "Invalid command line arguments, please use these inputs:"
	<< std::endl << std::endl << "a2p2 -s" << std::endl
	<< "a2p2 -c idNumber inputFile" << std::endl << std::endl;
	exit(EXIT_FAILURE);
}

//===============================================================================
// Utilities



//===============================================================================
// Submain/loops

void * client_cmd_send(std::fstream * fp, char * c2s_fifo)  {
	/* This function uses the io stream to send cmd lines to the server*/
	std::string line;
	while (std::getline(*fp, line))  {
		// Loop for grabbing cmd lines from the client file
		if (line[0] == '#')  {
			// Skipping if it's a comment
			continue;
		}
		std::cout << line << std::endl;
	}
}

//=============================================================================
// Main functions
void * server_main()  {
	// This function will serve as the main function for the server

	// Initilization
	int					idNumber = 0;
	std::cout << "Running the server" << std::endl;
	return NULL;
};

void * client_main(int idNumber, char * inputFile)  {
	// Variable initialization
	char 					c2s_fifo[10];
	std::fstream	fp(inputFile);
	// This function will serve as the main function for the client
	std::cout << "Running the client" << std::endl;

	// Need to create the fifo name
	sprintf(c2s_fifo, "fifo-0-%d", idNumber);
	std::cout << c2s_fifo << std::endl;
	// Making the fifo file
	mkfifo(c2s_fifo, 0666);
	client_cmd_send(&fp, c2s_fifo);

	return NULL;
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