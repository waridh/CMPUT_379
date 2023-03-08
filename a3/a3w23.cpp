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

//=============================================================================
// Main functions

void client_main(int argc, char * argv[])  {
  /* This is the main function for client */
  // TODO: Implement quit on the client

  // Initialization
  char            ip_name[30];
  int             cid;
  int             sfd[2];
  int             port_number;

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
  
  cid = atoi(argv[1]);
  port_number = atoi(argv[5]);

	// Socket initialization
  sfd[0] = client_connect(argv[4], port_number);


	


	std::cout << std::endl;

	
	// Closing all the pipes from this end
	close(sfd[0]);

	// Unlink the client specific pipes
	sleep(1);
	pthread_exit(NULL);
}

void server_main(int argc, char * argv[])  {
  // The main function for the server

  char                buffer[MSGSZ];
  int                 connected_clients = 0;
  int                 connections[NCLIENT] = {0};
  int                 i;
  int                 index;
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
        std::cout << "Smelled a connection" << std::endl;
        frominfolen = sizeof(frominfo);
        // Finding the index to put the thing in
        for (i = 0; i < NCLIENT; i++)  {
          // Checking for the first available index slot
          if (connections[i] == 0)  {
            connections[i] = 1;
            index = i;
          }
        }

        // Accepting the connection
        clientfds[index] = accept(
          pollfds[1].fd,
          (SA *) &frominfo,
          &frominfolen);
        // Adding the connection to the file descriptor list for poll
        pollfds[2 + index].fd = clientfds[index];
        pollfds[2 + index].events = POLLIN;
        pollfds[2 + index].revents = 0;
        connected_clients++;
      }

      for (i = 0; i < NCLIENT; i++)  {
        // The connected socket loop check
        if ((connections[i] == 1) && (pollfds[2 + i].revents & POLLIN))  {
          // Checking if there is input in this certain socket
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