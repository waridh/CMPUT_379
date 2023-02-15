#include <cstring>
#include <iostream>
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

//=============================================================================
void * server_main()  {
	// This function will serve as the main function for the server

	// Initilization
	int					idNumber = 0;
	std::cout << "Running the server" << std::endl;
	return NULL;
};

void * client_main(int idNumber, char * inputFile)  {
	// This function will serve as the main function for the client
	std::cout << "Running the client" << std::endl;
	std::cout << idNumber << std::endl;
	std::cout << inputFile << std::endl;
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