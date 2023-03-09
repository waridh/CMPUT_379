// THis program will do some tcp socket works

#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>


//=============================================================================
// Defining the sizes
#define   MAXWORD       32
#define   MSGSZ         64
#define   NCLIENT       3
#define   NOBJECT       16
#define   SA            struct sockaddr
#define   TOKSZ         3

//=============================================================================
// Global variables

clock_t                                 start = times(NULL); // For gtime
int                                     list_count = 0; // Counting list
static long                             clktck = 0; // Clk work
std::map<int, std::set<std::string>>    obj_list; // We are holding the objs

// Error checker for user input
void user_inpt_err(int argc, char * argv[])  {
  // Checking what is wrong
  if (argc < 3)  {
    std::cout << "Not enough arguments, please follow usage instructions"
    << std::endl;
    exit(EXIT_FAILURE);
  }  else if (argv[1][0] != '-')  {
    std::cout << "No flag present, please follow the running instructions"
    << std::endl;
    exit(EXIT_FAILURE);
  }  else if ((argv[1][1] != 's') && (argv[1][1] != 'c')) {
    std::cout << "Not a registered flag, please follow running instructions"
    << std::endl;
    exit(EXIT_FAILURE);
  }

}
//=============================================================================
// Commands
void delay_cmd(std::string delaytime)  {
  // Blocks the client for a certain time in milliseconds
  int         converted_time = stoi(delaytime);
	std::cout << "*** Entering a delay period of " << delaytime
			<< " msec" << std::endl;	
	usleep(converted_time*1000);
	std::cout << "*** Exiting delay period" << std::endl <<std::endl;
  return;
}

void quit_cmd(int fd)  {
	/* This is for when the server sends the quit cmd back to the client*/
	std::cout << "quitting" << std::endl;
	close(fd);
	exit(EXIT_SUCCESS);
}

void gtime_client(int fd)  {
  // Sending and recieving the gtime command
  write(fd, "GTIME", sizeof("GTIME"));
}

double gtime_cmd(int fd)  {
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

//=============================================================================
// Utilities

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

int client_connect(const char * ip_name, int port_number)  {
  // Connects the client to the server, and returns the file descriptor
  // Got a lot of these code from the eclass example
  // TODO: Implement IP lookup
  // TODO: Implement number input
  int                   ip_addr;
  int                   sfd;
  struct hostent        *hostinfo;
  struct sockaddr_in    server_addr;

  // Get host information
  hostinfo = gethostbyname(ip_name);
  if (hostinfo == (struct hostent *) NULL)  {
    std::cout << "Could not find the host through gethostbyname()"
    << std::endl;
  };
  
  // Convert the ip address to the desired value
  // TODO: For numerical input

  // put the host’s address, and type into a socket structure
  memset((char *) &server_addr, 0, sizeof(server_addr));
  memcpy((char *) &server_addr.sin_addr, hostinfo->h_addr, hostinfo->h_length);
  server_addr.sin_family= AF_INET;
  server_addr.sin_port= htons(port_number);
  // create a socket using TCP
  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    std::cout << "Client failed to create a socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  // Connecting to server
  if (connect(sfd, (SA *) &server_addr, sizeof(server_addr)) < 0)  {
    std::cout << "Client failed to connect to server" << std::endl;
    exit(EXIT_FAILURE);
  }

  return sfd;
}

int server_socket(int port_number)  {
  // Returns the file descriptor
  int                 sfd;
  struct sockaddr_in  sin;

  if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
    // Error checking for failed sockets creation
    std::cout << "Server failed to create a socket" << std::endl;
    exit(EXIT_FAILURE); 
  }

  // Initializing the stuff needed for socket
  memset((char *) & sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port = htons(port_number);

  // Binding the socket as a listener
  if (bind(sfd, (SA *) &sin, sizeof(sin)) < 0)  {
    // Could not bind the socket
    std::cout << "The server failed to bind the managing socket" << std::endl;
    exit(EXIT_FAILURE);
  }
  // Setting up amount of listeners
  listen(sfd, NCLIENT);
  std::cout << "Server set up" << std::endl;
  return sfd;
}

void server_poll_struct_set(struct pollfd pollstruct[])  {
  // Function is here to make the poll structure work with the server use.
  
  // The first fd is for stdin
  pollstruct[0].fd = STDIN_FILENO;
  pollstruct[0].events = POLLIN;
  pollstruct[0].revents = 0;

  // For the listening socket
  pollstruct[1].events = POLLIN;
  pollstruct[1].revents = 0;
}

int confirm_connection_client(int fd, int cid)  {
  // This function sends HELLO and the cid Returns of error
  char            buffer[MAXWORD];
  int             len;

  // Writing the HELLO packet
  strcpy(buffer, "HELLO");
  len = write(fd, buffer, sizeof(buffer));
  if (len == 0)  {
    std::cout << "Couldn't write HELLO packet. Quitting" << std::endl;
    exit(EXIT_FAILURE);
  }
  // Writing cid
  sprintf(buffer, "%d", cid);
  len = write(fd, buffer, sizeof(buffer));
  // Waiting for confirmation from server
  len = read(fd, buffer, sizeof(buffer));
  if (strncmp(buffer, "OK", 2) == 0)  {
    // Recieved the OK packet back from the server
    std::cout << "Recieved OK packet from server" << std::endl;
  }
  return 0;
}

int confirm_connection_server(int fd)  {
  // The server making sure the client sends a HELLO packet. Returns the CID
  char            buffer[MAXWORD];
  int             cid;
  int             len;
  // Reading buffer and then checking if it is appropriate
  len = read(fd, buffer, sizeof(buffer));
  if (strncmp(buffer, "HELLO", 5) == 0)  {
    // Checking packet
    len = read(fd, buffer, sizeof(buffer));
    cid = atoi(buffer);
    std::cout << "Recieved the HELLO packet from client " << cid << "."
    << std::endl;
    sprintf(buffer, "OK");
    write(fd, buffer, sizeof(buffer));
    return cid;
  }  else  {
    // Did not get the HELLO packet
    std::cout << "Did not recieve the HELLO packet, disconnecting" << std::endl;
    return -1;
  }

}

void client_transmitter(int fd, std::string * tokens)  {
  // Sending and recieving the thing without blocking
  if (tokens[1] == "delay")  {
    // First check if for delay
    delay_cmd(tokens[2]);
    return;
  }  else if (tokens[1] == "gtime")  {
    // Asking for the time since server began
  }
}

void server_reciever(int fd)  {
  // This function will recieve the packets from the client
  char        buffer[MAXWORD];
  int         len;

  len = read(fd, buffer, sizeof(buffer));

}




//=============================================================================
// Main functions

void client_sendcmds(int fd, int cid, char * filename)  {
  /* This command will read from the specified file and then send those to the
  fd specified. */
  char            buff0[MAXWORD];
  std::fstream   fp(filename);
  std::string     cmdline;
  std::string     tokens[TOKSZ];
  std::cout << filename << std::endl;
  while (std::getline(fp, cmdline))  {
		// Loop for grabbing cmd lines from the client file
    if (
      (cmdline[0] == '#')
      || (cmdline[0] == '\n')
      || (cmdline.size() == 0)
      || (cmdline[0] != '0' + cid)
    )  {
      // Skipping if it's a comment, or line is empty, or if it's not for client
      continue;
    }
    strcpy(buff0, cmdline.c_str());
    tokenizer(buff0, tokens);
    std::cout << cmdline << std::endl;
    client_transmitter(fd, tokens);
  }

  return;
}

void client_main(int argc, char * argv[])  {
  /* This is the main function for client */
  // TODO: Implement quit on the client

  // Initialization
  char            ip_name[30];
  char            buffer[MAXWORD];
  int             cid;
  int             port_number;
  struct pollfd   pollfds[2];

  // Last input client check
  if (argc != 6)  {
    // not enough arguments
    std::cout << "Wrong amount of arguments for client, consider following" <<
    " running instructions" << std::endl;

    exit(EXIT_FAILURE);
  }  else if (atoi(argv[2]) > NCLIENT)  {
    // Wrong cid error handling
    std::cout << "The Client ID provided is out of range" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Setting up the pollfd
  server_poll_struct_set(pollfds);
  
  cid = atoi(argv[2]);
  port_number = atoi(argv[5]);

	// Socket initialization
  pollfds[1].fd = client_connect(argv[4], port_number);

  // Confirming connections with the server 
  confirm_connection_client(pollfds[1].fd, cid);

  // Need to loop and read the commands from the file and send it to the socket
  client_sendcmds(pollfds[1].fd, cid, argv[3]);

	
	// Closing all the pipes from this end
	close(pollfds[1].fd);

	// Unlink the client specific pipes
	sleep(1);
	pthread_exit(NULL);
}

void server_main(int argc, char * argv[])  {
  // The main function for the server

  char                buffer[MAXWORD];
  int                 buffd;
  int                 cid;
  int                 connected_clients = 0;
  int                 connections[NCLIENT] = {0};
  int                 i;
  int                 index;
  int                 len;
  int                 sid = 0;
  int                 sfd;
  int                 clientfds[NCLIENT];
  int                 port_number;
  int                 timeout = 100;
  struct pollfd       pollfds[2 + NCLIENT];
  struct sockaddr_in  frominfo;
  socklen_t           frominfolen;

  // Final error check
  if (argc != 3)  {
    // Not the right amount of arguments
    std::cout << "There isn't the correct amount of arguments for this program"
    << std::endl;
    exit(EXIT_FAILURE);
  }

  // Initialize the server address and port for the server
  port_number = atoi(argv[2]);

  // Setting up the listening socket
  pollfds[1].fd = server_socket(port_number);

  // Setting up the poll structure
  server_poll_struct_set(pollfds);

  while (1)  {
    // The main loop for server. Bits of the code was inspired by the eclass
    if (poll(pollfds, 2 + NCLIENT, timeout) > 0)  {
      // Polling for any inputs
      if ((pollfds[0].revents & POLLIN) != 0)  {

				if (fgets(buffer, MSGSZ, stdin))  {
					if (strncmp(buffer, "quit", 4) == 0)  {
						std::cout << "Quitting" << std::endl;
            exit(EXIT_SUCCESS);
						
					}  else if (strncmp(buffer, "list", 4) == 0)  {
						std::cout << "List placeholder" << std::endl;
						
					}
				}
      }  if ((connected_clients < NCLIENT) && (pollfds[1].revents & POLLIN))  {
        // Taking a new connection
        frominfolen = sizeof(frominfo);
        // Finding the index to put the thing in

        // Accepting the connection
        buffd = accept(
          pollfds[1].fd,
          (SA *) &frominfo,
          &frominfolen
        );

        cid = confirm_connection_server(buffd);  // Pain
        if (connections[cid - 1] != 0)  {
          // There is already a client with this ID
          std::cout << "A connection with the same client ID already exists"
          << std::endl;

          // Need to forget about this cid, so I guess I will jsut continue the
          // loop
          continue;
        }  else  {
          // Setting the fd to this array too
          connections[cid - 1] = cid;
        }

        // Adding the connection to the file descriptor list for poll
        pollfds[1 + cid].fd = buffd;
        pollfds[1 + cid].events = POLLIN;
        pollfds[1 + cid].revents = 0;
        connected_clients++;
      }

      for (i = 0; i < NCLIENT; i++)  {
        // The connected socket loop check
        if ((connections[i] != 0) && (pollfds[2 + i].revents & POLLIN))  {
          // Checking if there is input in this certain socket
          len = read(pollfds[2 + i].fd, buffer, sizeof(buffer));
          if (len == 0)  {
            // TODO: Implement the client number that we lost connection with
            std::cout << "lost connection to client " << connections[i]
            << std::endl;
            connections[i] = 0;
            continue;
          }
          std::cout << buffer << std::endl;
          write(pollfds[2 + i].fd, buffer, sizeof(buffer));
        }
      }
    }
  }

  return;
}

int main(int argc, char * argv[])  {
  // The main function
  user_inpt_err(argc, argv);

  if (argv[1][1] == 'c')  {
    // Running the client
    client_main(argc, argv);
  } else if (argv[1][1] == 's')  {
    // Running the server
    server_main(argc, argv);
  }
  return 0;
}