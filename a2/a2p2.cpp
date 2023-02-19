#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define		ARGAMT						3
#define 	NCLIENT 					3
#define		BUFFERSZ 					3
#define		MAXWORD						32
#define		MSGSZ							512

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

typedef struct  {
	// For each object
	int						cid;
	char					objName[MAXWORD];
} single_obj;

//=============================================================================
// Global variables

int							list_count = 0;
single_obj			the_list[32];
time_t					start = time(0);

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
// Tokenization
void tokenizer(char * cmdline, std::string * tokens)  {
	/* Goal here is to tokenize the c string input*/
	char							WSPACE[] = "\t ";
	char *						buffer = strtok(cmdline, WSPACE);
	int								count = 0;
	while (buffer != NULL)  {
		// Putting strings down
		tokens[count] = buffer;
		count++;
		buffer = strtok(NULL, WSPACE);
	}
};

//=============================================================================
// Cmd functions

void quit_cmd()  {
	/* This is for when the server sends the quit cmd back to the client*/
	std::cout << "quitting" << std::endl;
	exit(EXIT_SUCCESS);
}

double gtime_cmd()  {
	double						seconds_passed = difftime(time(0), start);
	return seconds_passed;
}

void delay_cmd(const char * delaytime)  {
	/* This is the delay command. Takes in milliseconds*/
	int								converted_time = atoi(delaytime);
	std::cout << "Delaying" << std::endl;
	usleep(converted_time*1000);
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

void send_id2s(int idNumber)  {
	/* The goal of this function is to send the integer cid to the server*/
	char *					init_fifo = "fifo-to-0";
	char						idmsg[BUFFERSZ];
	int							fd;
	fd = open(init_fifo, O_WRONLY);
	sprintf(idmsg, "%d", idNumber);

	write(fd, idmsg, BUFFERSZ + 1);	

	close(fd);
}

void print_transmitted(char * src, char * packet_inf)  {
	/* This function prints info about the sent packet*/
	std::cout << "Transmitted (src= " << src << ") " << packet_inf << std::endl;
}

void print_received(char * src, char * packet_inf)  {
	/* Prints info about the packet that was recieved*/
	std::cout << "Received (src= " << src << ") " << packet_inf << std::endl;
}

//=============================================================================
// Submain/loops

void * client_cmd_send(void * arg)  {
	/* This function uses the io stream to send cmd lines to the server*/
	csend_t *				csend = (csend_t *) arg;
	char						buffer[MSGSZ];
	char						cid[2];
	char						packet_type[10];
	char						s2c_fifo[10];
	char						src_self[10];
	int 						fd;
	std::fstream		fp(csend->inFile);
	std::string 		line;
	std::string			tokens[ARGAMT];
	wfifo_t					wfifo;

	sprintf(cid, "%d", csend->cid);
	sprintf(src_self, "client:%s", cid);
	sprintf(s2c_fifo, "fifo-0-%s", cid);

	std::cout << "Connecting to server" << std::endl;
	std::cout << csend->c2s_fifo << std::endl;
	fd = open(csend->c2s_fifo, O_WRONLY);
	std::cout << "Connected to server" << std::endl;

	wfifo.fifo_n = csend->c2s_fifo;
	while (std::getline(fp, line))  {
		// Loop for grabbing cmd lines from the client file
		if ((line[0] == '#')
		|| (line[0] == '\n')
		|| (line.size() == 0)
		|| (line[0] != cid[0])
		)  {
			// Skipping if it's a comment, or line is empty
			continue;
		}
		strcpy(buffer, line.c_str());
		// Switch cases for the file inputs
		tokenizer(buffer, tokens);
		if (strncmp(tokens[1].c_str(), "gtime", 5) == 0)  {
			strcpy(packet_type, "GTIME");
			print_transmitted(src_self, packet_type);
			write(fd, packet_type, MSGSZ);
		}  else if (strncmp(tokens[1].c_str(), "delay", 5) == 0)  {
			strcpy(packet_type, "DELAY");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			print_transmitted(src_self, packet_type);
		}  else if (strncmp(tokens[1].c_str(), "put", 3) == 0)  {
			strcpy(packet_type, "PUT");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			print_transmitted(src_self, packet_type);
		}  else if (strncmp(tokens[1].c_str(), "get", 3) == 0)  {
			strcpy(packet_type, "GET");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			print_transmitted(src_self, packet_type);
		}  else if (strncmp(tokens[1].c_str(), "delete", 6) == 0)  {
			strcpy(packet_type, "DELETE");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			print_transmitted(src_self, packet_type);
		}  else if (strncmp(tokens[1].c_str(), "quit", 4) == 0)  {
			strcpy(packet_type, "QUIT");
			write(fd, packet_type, MSGSZ);
		}
		
	}
	std::cout << "GOT OUT OF LOOP" << std::endl;
	close(fd);
	// unlink(csend->c2s_fifo);
	sleep(1);
	pthread_exit(NULL);
}

void * client_reciever(void * arg)  {
	/* This function will be used with a thread to take inputs*/
	csend_t *					crecieve = (csend_t *) arg;
	char							buffer[MSGSZ];
	int								fd;

	std::cout << "Recieving from server" << std::endl;
	std::cout << crecieve->c2s_fifo << std::endl;
	fd = open(crecieve->c2s_fifo, O_RDONLY);
	while (1)  {
		if (read(fd, buffer, MSGSZ) > 0)  {
			std::cout << buffer << std::endl;
			if (strncmp(buffer, "QUIT", 4) == 0)  {
				std::cout << buffer << std::endl;
				break;
			}
		}
	}
	std::cout << "EXITED R LOOP" << std::endl;
	close(fd);
	unlink(crecieve->c2s_fifo);
}

void * server_connect(void * arg)  {
	/* This takes the client id, and connect this thread to the pipe*/
	char *					cid = (char *) arg;
	char						buffer[MSGSZ];
	char						packet_type[10];
	char						pipe_in[10];
	char						pipe_out[10];
	char						src[10];
	char						returnmsg[MSGSZ];
	double					gtimer;
	int							fdi;
	int							fdo;
	std::string			tokens[ARGAMT];

	// Getting the names of the pipes
	std::cout << "Connecting to pipes" << std::endl;
	sprintf(pipe_in, "fifo-%s-0", cid);
	sprintf(pipe_out, "fifo-0-%s", cid);
	sprintf(src, "client:%s", cid);

	// Opening both pipes
	std::cout << pipe_in << std::endl;
	fdi = open(pipe_in, O_RDONLY);
	std::cout << "Opened the in pipe" << std::endl;
	fdo = open(pipe_out, O_WRONLY);
	std::cout << "Opened the out pipe" << std::endl;

	while (read(fdi, buffer, MSGSZ) > 0)  {
		// Reading the cmds from client.
		strcpy(packet_type, buffer);
		write(fdo, buffer, MSGSZ);

		// Server task switch. Independent function doesn't have enough info
		if (strncmp(packet_type, "GTIME", 5) == 0)  {
			// For gtime
			print_received(src, packet_type);
			gtimer = gtime_cmd();
			sprintf(returnmsg, "TIME %0.2f", gtimer);
			write(fdo, returnmsg, MSGSZ);

		}  else if (strncmp(packet_type, "DELAY", 5) == 0)  {
			// This is the delay function
			print_received(src, packet_type);
			if (read(fdi, buffer, MSGSZ) > 0)  {
				std::cout << buffer << std::endl;
				delay_cmd(buffer);
			};
			
		}  else if (strncmp(packet_type, "QUIT", 4) == 0)  {
			// This is the delay function
			write(fdo, packet_type, MSGSZ);
			
		} else if (strncmp(packet_type, "PUT", 3) == 0)  {
			// This is the delay function
			print_received(src, packet_type);
			if (read(fdi, buffer, MSGSZ) > 0)  {
				std::cout << buffer << std::endl;
			};
			
		}else if (strncmp(packet_type, "GET", 3) == 0)  {
			// This is the delay function
			print_received(src, packet_type);
			if (read(fdi, buffer, MSGSZ) > 0)  {
				std::cout << buffer << std::endl;
			};
			
		}else if (strncmp(packet_type, "DELETE", 6) == 0)  {
			// This is the delay function
			print_received(src, packet_type);
			if (read(fdi, buffer, MSGSZ) > 0)  {
				std::cout << buffer << std::endl;
			};
			
		};

		list_count++;
	}
	std::cout << "OUT OF LOOP" << std::endl;
	close(fdi);
	close(fdo);
	unlink(pipe_in);
	pthread_exit(NULL);

}

void * server_reciever(void * arg)  {
	/* This thread function will loop and read inputs */
	// Current idea. I will make this thread create a pipe that just takes ints
	// from clients so that it knows to connect to what pipe.
	char *					init_fifo = "fifo-to-0";
	char						buffer[BUFFERSZ];
	int							fd1;
	pthread_t				tidc;
	unsigned int		count = 0;
	mkfifo(init_fifo, 0666);  // Figure out what the number means
	fd1 = open(init_fifo, O_RDONLY);

	while (1) {
		
		// Wait for a signal
		read(fd1, buffer, BUFFERSZ);
		//std::cout << strlen(buffer) << std::endl;
		if ((strlen(buffer) != 0) && (buffer != "\n"))  {
			std::cout << buffer << std::endl;
			pthread_create(
				&tidc,
				NULL,
				server_connect,
				(void *) buffer
			);
			pthread_join(tidc, NULL);
		}
		
		
	}
	close(fd1);
	unlink(init_fifo);
}

//=============================================================================
// Main functions
void server_main()  {
	// This function will serve as the main function for the server

	// Initilization
	char				line[1024];
	fd_set			fds;
	int					idNumber = 0;
	pthread_t		tid1;
	std::cout << "a2p2: do_server" << std::endl;

	pthread_create(
		&tid1,
		NULL,
		server_reciever,
		NULL
	);

	while (1)  {
		/* Testing */
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		select(STDIN_FILENO+1, &fds, NULL, NULL, NULL);

		if (FD_ISSET(STDIN_FILENO, &fds))  {
			// If we catch an input
			if (fgets(line, 1024, stdin))  {
				if (strncmp(line, "quit", 4) == 0)  {
					quit_cmd();
				}  else if (strncmp(line, "list", 4) == 0)  {
					std::cout << "LIST PLACEHOLDER" << std::endl;
					std::cout << list_count << std::endl;
				}
			}
		}
	}

	pthread_join(tid1, NULL);

	return;
};

void client_main(int idNumber, char * inputFile)  {
	// Variable initialization
	char 						c2s_fifo[10];
	char						id2ser[BUFFERSZ];
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
	sprintf(s2c_fifo, "fifo-0-%d", idNumber);
	sprintf(c2s_fifo, "fifo-%d-0", idNumber);
	csend.c2s_fifo = c2s_fifo;
	crecieve.c2s_fifo = s2c_fifo;
	// Making the fifo file
	mkfifo(csend.c2s_fifo, 0666);
	mkfifo(crecieve.c2s_fifo, 0666);
	// Connecting to the server
	send_id2s(idNumber);
	// Fully initialize the struct being sent to the thread
	csend.inFile = inputFile;
	csend.cid = idNumber;
	// Creating thread for sending data to the server
	std::cout << "CREATEING THREADS" << std::endl;
	if (pthread_create(
		&tids,
		NULL,
		client_cmd_send,
		(void *) &csend
	) != 0)  {
		// Thread creation failure handling
		thread_create_fail();
	};
	if (pthread_create(
		&tidr,
		NULL,
		client_reciever,
		(void *) &crecieve
	) != 0)  {
		// Thread creation failure handling
		thread_create_fail();
	};
	pthread_join(tids, NULL);
	pthread_join(tidr, NULL);


	
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