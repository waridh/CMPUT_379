#include <bits/stdc++.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>
#include <sys/times.h>
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


//=============================================================================
// Global variables

clock_t																	start = times(NULL);
int																			list_count = 0;
static long															clktck = 0;
std::map<int, std::set<std::string>>		obj_list;


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
	unlink("fifo-to-0");
	exit(EXIT_SUCCESS);
}

double gtime_cmd()  {
	clock_t						current_time = times(NULL);
	clock_t						c_seconds_passed = current_time - start;
	double						seconds_passed;

	if (clktck == 0)  {
		clktck = sysconf(_SC_CLK_TCK);
		if (clktck < 0)  {
			// Something broke
			std::cout << "sysconf error" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	seconds_passed = c_seconds_passed / (double) clktck;
	return seconds_passed;
}

int put_cmd(char * cid, char * item)  {
	/* This function adds an object to the list*/
	std::string				item_s = item;
	int								cidi = atoi(cid);
	if (obj_list[cidi].find(item_s) != obj_list[cidi].end())  {
		// Already exists, return an error
		return -1;
	}  else  {
		obj_list[cidi].insert(item_s);
		return 0;
	}
}

void delay_cmd(const char * delaytime)  {
	/* This is the delay command. Takes in milliseconds*/
	int								converted_time = atoi(delaytime);
	std::cout << "*** Entering a delay period of " << delaytime
			<< " msec" << std::endl;	
	usleep(converted_time*1000);
	std::cout << "*** Exiting delay period" << std::endl <<std::endl;
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

void pthread_cleanup_handler(void * arg)  {
	/* Makes sure to close and unlink the receiver for the server*/
	int *						fd = (int *) arg;
	close(*fd);
	unlink("fifo-to-0");
}

void output_list()  {
	/* This function makes the server output the contents of the object list*/
	std::cout << "Object table:" << std::endl;

	for (auto i : obj_list)  {
		std::cout << i.first << std::endl;
		for (auto j : i.second)  {
			std::cout << "(owner= " << i.first << ", name= " << j << ")"
			<< std::endl;
		}
	}
	return;
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
	char						src_server[7] = "server";
	int 						fd;
	int							fdr;
	std::fstream		fp(csend->inFile);
	std::string 		line;
	std::string			tokens[ARGAMT];
	

	sprintf(cid, "%d", csend->cid);
	sprintf(src_self, "client:%s", cid);
	sprintf(s2c_fifo, "fifo-0-%s", cid);

	fd = open(csend->c2s_fifo, O_WRONLY);
	fdr = open(s2c_fifo, O_RDONLY);
	std::cout << std::endl;

	
	while (std::getline(fp, line) || 1)  {
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
			
			delay_cmd(tokens[2].c_str());
			
			continue;
		}  else if (strncmp(tokens[1].c_str(), "put", 3) == 0)  {
			// This switch handles PUT input.
			strcpy(packet_type, "PUT");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);

		}  else if (strncmp(tokens[1].c_str(), "get", 3) == 0)  {
			strcpy(packet_type, "GET");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);
		}  else if (strncmp(tokens[1].c_str(), "delete", 6) == 0)  {
			strcpy(packet_type, "DELETE");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);
		}  else if (strncmp(tokens[1].c_str(), "quit", 4) == 0)  {
			strcpy(packet_type, "QUIT");
			write(fd, packet_type, MSGSZ);
		}

		read(fdr, buffer, MSGSZ);
		if (strncmp(buffer, "QUIT", 4) == 0)  {
			// Quit command
			break;
		}  else if (strncmp(buffer, "TIME", 4) == 0)  {
			// TIME packet
			read(fdr, packet_type, MSGSZ);
			sprintf(buffer, "(TIME:\t%s)", packet_type);
			print_received(src_server, buffer);
		}  else  {
			print_received(src_server, buffer);
		}
		
		std::cout << std::endl;
	}

	close(fd);
	close(fdr);
	unlink(s2c_fifo);
	sleep(1);
	pthread_exit(NULL);
}

void * server_connect(void * arg)  {
	/* This takes the client id, and connect this thread to the pipe*/
	char *					cid = (char *) arg;
	char						buffer[MSGSZ];
	char						packet_type[10];
	char						pipe_in[10];
	char						pipe_out[10];
	char						src[10];
	char						src_self[7] = "server";
	char						returnmsg[MSGSZ];
	char						word[MAXWORD];
	double					gtimer;
	int							err_flag;
	int							fdi;
	int							fdo;
	std::string			tokens[ARGAMT];

	// Getting the names of the pipes

	sprintf(pipe_in, "fifo-%s-0", cid);
	sprintf(pipe_out, "fifo-0-%s", cid);
	sprintf(src, "client:%s", cid);

	// Opening both pipes
	
	fdi = open(pipe_in, O_RDONLY);	
	fdo = open(pipe_out, O_WRONLY);

	while (read(fdi, buffer, MSGSZ) > 0)  {
		// Reading the cmds from client.
		strcpy(packet_type, buffer);
		sprintf(returnmsg, "OK");
		// Server task switch. Independent function doesn't have enough info
		if (strncmp(packet_type, "GTIME", 5) == 0)  {
			// For gtime
			print_received(src, packet_type);
			gtimer = gtime_cmd();
			// Sending the TIME packet
			strcpy(packet_type, "TIME");
			sprintf(buffer, "%0.2f", gtimer);
			write(fdo, packet_type, MSGSZ);
			write(fdo, buffer, MSGSZ);
			// Transmitted msg
			sprintf(returnmsg, "(TIME:\t%0.2f)", gtimer);
			print_transmitted(src_self, returnmsg);
			
		}  else if (strncmp(packet_type, "QUIT", 4) == 0)  {
			// This is the delay function
			write(fdo, packet_type, MSGSZ);
			break;
			
		} else if (strncmp(packet_type, "PUT", 3) == 0)  {
			// This is the switch for put
			
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the thing
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				err_flag = put_cmd(cid, word);
				if (err_flag == 0)  {
				sprintf(returnmsg, "OK");
				write(fdo, returnmsg, MSGSZ);
				}  else  {
					// Error handling
				}
				print_transmitted(src_self, returnmsg);
				

			};
			
			
			
		}  else if (strncmp(packet_type, "GET", 3) == 0)  {
			// This is the delay function
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the thing
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				sprintf(returnmsg, "OK");
				write(fdo, returnmsg, MSGSZ);
				print_transmitted(src_self, returnmsg);

			};
		}  else if (strncmp(packet_type, "DELETE", 6) == 0)  {
			// This is the delay function
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the thing
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				sprintf(returnmsg, "OK");
				write(fdo, returnmsg, MSGSZ);
				print_transmitted(src_self, returnmsg);

			};
		};
		std::cout << std::endl;
		list_count++;
	}
	// Closing the pipes with this client
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
	fd_set					fds;
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
	pthread_exit(NULL);
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
					output_list();
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