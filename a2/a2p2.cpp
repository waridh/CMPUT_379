#include <bits/stdc++.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <poll.h>
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
#define		FIFONAMESZ				10
#define		MAXWORD						32
#define		MSGSZ							64
#define		CIDSZ							3

//=============================================================================
// Structs

typedef struct  {
	// Struct for the send thread
	char * 				inFile;
	char * 				c2s_fifo;
	int						cid;
} csend_t;


//=============================================================================
// Global variables

clock_t																	start = times(NULL); // For gtime
int																			list_count = 0; // Counting list
static long															clktck = 0; // Clk work
std::map<int, std::set<std::string>>		obj_list; // We are holding the objs


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
	// Self explanatory
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

void quit_cmd(int fd)  {
	/* This is for when the server sends the quit cmd back to the client*/
	std::cout << "quitting" << std::endl;
	close(fd);
	unlink("fifo-to-0"); // Also deletes the select pipe
	exit(EXIT_SUCCESS);
}

double gtime_cmd()  {
	/* Gets the time since start. Takes advantage of times()*/
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

int get_cmd(char * cid, char * item)  {
	/* This function retrieves the object from the list*/
	std::string				item_s = item;
	for (auto i : obj_list)  {
		/* Need to look at all the objects what was put here by all clients*/
		if (i.second.find(item_s) != i.second.end())  {
			/* If the object exists, then do normal return*/
			return 0;
		}
	}
	/* The object does not exist in the thing, return error*/
	return -1;
}

int delete_cmd(char * cid, char * item)  {
	/* This function will delete the object that is stored*/
	std::string				item_s = item;
	int								cidi = atoi(cid);
	if (obj_list[cidi].find(item_s) != obj_list[cidi].end())  {
		/* If the object belongs to client, proceed as intended*/
		obj_list[cidi].erase(item_s);
		return 0;
	}
	for (auto i : obj_list)  {
		/* Need to look at all the objects what was put here by all clients*/
		if (i.first == cidi)  {
			continue;
		}  else if (i.second.find(item_s) != i.second.end())  {
			/* If the object exists, but not owned by client*/
			return -2;
		}
	}
	/* The object does not exist in the thing*/
	return -1;
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
	if (list_count == 0)  {
		std::cout << "list is empty" << std::endl;
	}
	for (auto i : obj_list)  {
		
		for (auto j : i.second)  {
			std::cout << "(owner= " << i.first << ", name= " << j << ")"
			<< std::endl;
		}
	}
	std::cout << std::endl;
	return;
}

//=============================================================================
// Submain/loops

void * client_cmd_send(void * arg)  {
	/* This function uses the io stream to send cmd lines to the server*/
	csend_t *				csend = (csend_t *) arg;
	char						buffer[MSGSZ];
	char						msg[MSGSZ];
	char						cid[CIDSZ];
	char						ecid[MSGSZ];
	char						packet_type[10];
	char						pipe_mux[10];
	char						s2c_fifo[10];
	char						src_self[10];
	char						src_server[7] = "server";
	char						word[MAXWORD];
	int							fdm;
	int 						fd;
	int							fdr;
	int							switcher = 0;
	std::fstream		fp(csend->inFile);
	std::string 		line;
	std::string			tokens[ARGAMT];
	
	// Getting the names of the pipes and cid for selector
	sprintf(cid, "%d", csend->cid);
	sprintf(pipe_mux, "fifo-to-0");
	sprintf(s2c_fifo, "fifo-0-%s", cid);
	
	// Waking up the server	
	fdm = open(pipe_mux, O_WRONLY | O_NONBLOCK);
	write(fdm, cid, CIDSZ);

	// Establishing connection with the server via opening both pipes	
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
			// Skipping if it's a comment, or line is empty, or if it's not for client
			continue;
		}

		// For some reason, the id for itself is changing
		sprintf(src_self, "client:%s", cid);
		strcpy(buffer, line.c_str());
		// Switch cases for the file inputs
		tokenizer(buffer, tokens);
		
		if (strncmp(tokens[1].c_str(), "delay", 5) == 0)  {
			// Delay is placed here to prevent waking up the server.
			delay_cmd(tokens[2].c_str());
			
			continue;
		}

		if (switcher == 1)  {
			// Continue to send the select signal to the server so it can do mux
			write(fdm, cid, CIDSZ);
		}
			
		if (strncmp(tokens[1].c_str(), "gtime", 5) == 0)  {
			// Sending the gtime function
			strcpy(packet_type, "GTIME");
			write(fd, packet_type, MSGSZ);
			print_transmitted(src_self, packet_type);
		}  else if (strncmp(tokens[1].c_str(), "put", 3) == 0)  {
			// This switch handles PUT input.
			strcpy(packet_type, "PUT");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);

		}  else if (strncmp(tokens[1].c_str(), "get", 3) == 0)  {
			// Sending the get packet over.
			strcpy(packet_type, "GET");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);
		}  else if (strncmp(tokens[1].c_str(), "delete", 6) == 0)  {
			// Sending DELETE packet
			strcpy(packet_type, "DELETE");
			write(fd, packet_type, MSGSZ);
			write(fd, tokens[2].c_str(), MSGSZ);
			sprintf(buffer, "(%s: %s)", packet_type, tokens[2].c_str());
			print_transmitted(src_self, buffer);
		}  else if (strncmp(tokens[1].c_str(), "quit", 4) == 0)  {
			// Informing the server that we are quitting the client. Needs responds
			strcpy(packet_type, "QUIT");
			write(fd, packet_type, MSGSZ);
		}

		read(fdr, packet_type, MSGSZ);
		if (strncmp(packet_type, "QUIT", 4) == 0)  {
			// Quit command
			break;
		}  else if (strncmp(packet_type, "TIME", 4) == 0)  {
			// TIME packet
			read(fdr, word, MSGSZ);
			sprintf(msg, "(TIME:\t%s)", word);
			print_received(src_server, msg);
		}  else if (strncmp(packet_type, "ERROR", 5) == 0)  {
			// Error handling. Triggered on ERROR packet type
			read(fdr, word, MSGSZ);
			sprintf(msg, "(%s: %s)", packet_type, word);
			print_received(src_server, msg);
		}  else  {
			print_received(src_server, packet_type);
		}
		std::cout << std::endl;
		switcher = 1;
	}

	// Making sure that we have the closing pipenames ready
	sprintf(s2c_fifo, "fifo-0-%s", cid);
	sprintf(ecid, "-%s", cid);
	write(fdm, ecid, MSGSZ);
	
	// Closing all the pipes from this end
	close(fd);
	close(fdr);
	close(fdm);

	// Unlink the client specific pipes
	unlink(s2c_fifo);
	unlink(csend->c2s_fifo);
	sleep(1);
	pthread_exit(NULL);
}

void server_connect(char * cid, int fdi, int fdo)  {
	/* This function does the two way communication with the client every time
	the select signal is recieved. It has all the cases in the function, so it
	will have a proper response for all the packets*/
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
	std::string			tokens[ARGAMT];

	// Getting the names of the pipes

	sprintf(pipe_in, "fifo-%s-0", cid);
	sprintf(pipe_out, "fifo-0-%s", cid);
	sprintf(src, "client:%s", cid);

	if (read(fdi, buffer, MSGSZ) > 0)  {
		// Reading the cmds from client.
		
		strcpy(packet_type, buffer);
		sprintf(returnmsg, "OK");
		if (strncmp(packet_type, "QUIT", 4) == 0)  {
			// When QUIT is received, we don't want to do much else. Just return
			write(fdo, packet_type, MSGSZ);
			return;
			
		}
		// For formatting
		std::cout << std::endl;
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
			
		}   else if (strncmp(packet_type, "PUT", 3) == 0)  {
			// This is the switch for put
			
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the message
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				err_flag = put_cmd(cid, word);
				if (err_flag == 0)  {
					sprintf(returnmsg, "OK");
					write(fdo, returnmsg, MSGSZ);
					list_count++;
				}  else  {
					// Error handling
					sprintf(packet_type, "ERROR");
					write(fdo, packet_type, MSGSZ);
					write(fdo, "object already present", MSGSZ);
					sprintf(returnmsg, "(ERROR: object already present)");
				}
				print_transmitted(src_self, returnmsg);
				
			};
				
		}  else if (strncmp(packet_type, "GET", 3) == 0)  {
			// Handling for the GET packet
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the thing
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				err_flag = get_cmd(cid, word);
				if (err_flag == 0)  {
					// Normal return
					sprintf(returnmsg, "OK");
					write(fdo, returnmsg, MSGSZ);

				}  else  {
					// Did not find the object
					sprintf(packet_type, "ERROR");
					write(fdo, packet_type, MSGSZ);
					write(fdo, "object not found", MSGSZ);
					sprintf(returnmsg, "(ERROR: object not found)");
				}
				
				print_transmitted(src_self, returnmsg);

			};
		}  else if (strncmp(packet_type, "DELETE", 6) == 0)  {
			// DELETE packet handling
			if (read(fdi, word, MSGSZ) > 0)  {
				// If we were able to read the thing
				sprintf(returnmsg, "(%s: %s)", packet_type, word);
				print_received(src, returnmsg);
				err_flag = delete_cmd(cid, word);
				if (err_flag == 0)  {
					// Normal return
					sprintf(returnmsg, "OK");
					write(fdo, returnmsg, MSGSZ);
					list_count--;

				}  else if (err_flag == -1)  {
					// Did not find the object
					sprintf(packet_type, "ERROR");
					write(fdo, packet_type, MSGSZ);
					write(fdo, "object not found", MSGSZ);
					sprintf(returnmsg, "(ERROR: object not found)");
				}  else  {
					// Object does not belong to client
					sprintf(packet_type, "ERROR");
					write(fdo, packet_type, MSGSZ);
					write(fdo, "client not owner", MSGSZ);
					sprintf(returnmsg, "(ERROR: client not owner)");
				}
				
				print_transmitted(src_self, returnmsg);
			};
		};
	}
	
	return;
}

//=============================================================================
// Main functions
void server_main()  {
	// This function will serve as the main function for the server

	// Initilization
	char					cid[CIDSZ];
	char					fifo_name[FIFONAMESZ];
	char					line[1024];
	const char *	initfifo = "fifo-to-0";
	int						cidi;
	int						ecid;
	int						fdi;
	int						mux_switchesi[NCLIENT] = {0};
	int						mux_switcheso[NCLIENT] = {0};
	int						timeout = 100;
	struct pollfd	ufds[2];
	

	// Formatting texts
	std::cout << "a2p2: do_server" << std::endl;

	mkfifo(initfifo, 0666);
	fdi = open(initfifo, O_RDONLY | O_NONBLOCK);

	// Prep for polling
	ufds[0].fd = 0;
	ufds[0].events = POLLIN;
	ufds[0].revents = 0;
	ufds[1].fd = fdi;
	ufds[1].events = POLLIN;


	while (1)  {
		/* Polling for both user input and mux fifo */
		
		if (poll(ufds, 2, timeout) > 0)  {
			// Checking the stdin
			if ((ufds[0].revents & POLLIN) != 0)  {

				if (fgets(line, 1024, stdin))  {
					if (strncmp(line, "quit", 4) == 0)  {
						quit_cmd(fdi);
						
					}  else if (strncmp(line, "list", 4) == 0)  {
						output_list();
						
					}
				}
			} if ((ufds[1].revents & POLLIN) != 0)  {
				// Looking at the select signal
				if (read(ufds[1].fd, cid, CIDSZ) > 0)  {
					// If we were able to read from the selector switch	
					cidi = atoi(cid);
					if ((cidi == 0) || (cidi > NCLIENT))  {
						continue;
					}  else if  (cidi < 0)  {
						// A client has closed
						ecid = cidi * -1;
						// Closing the required pipes
						close(mux_switchesi[ecid -1]);
						close(mux_switcheso[ecid - 1]);
						
						mux_switchesi[ecid - 1] = 0;
						mux_switcheso[ecid - 1] = 0;
						
					}  else if (mux_switchesi[cidi-1] != 0)  {
						// Plan is to call the cases function right here
						server_connect(cid, mux_switchesi[cidi-1], mux_switcheso[cidi-1]);
					}  else  {
						// A new connection has been made, we are going to open the pipe
						sprintf(fifo_name, "fifo-%s-0", cid);
						mux_switchesi[cidi - 1] = open(fifo_name, O_RDONLY);
						sprintf(fifo_name, "fifo-0-%s", cid);
						mux_switcheso[cidi - 1] = open(fifo_name, O_WRONLY);
						server_connect(cid, mux_switchesi[cidi-1], mux_switcheso[cidi-1]);
					}
				}
			}
		}
	}

	close(fdi);
	return;
};

void client_main(int idNumber, char * inputFile)  {
	// Variable initialization
	char 						c2s_fifo[10];
	char						s2c_fifo[10];
	csend_t					csend;
	csend_t					crecieve;
	pthread_t				tids;
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