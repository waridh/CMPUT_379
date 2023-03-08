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
  // TODO: Implement IP lookup
  // TODO: Implement number input
  int                   ip_addr;
  int                   sfd;
  struct hostent        *hostinfo;
  struct sockaddr_in    server_addr;

  // Get host information
  hostinfo = gethostbyname(ip_name);
  if (hostinfo == (struct hostent * NULL))  {
    std::cout << "Could not find the host through gethostbyname()"
    << std::endl;
  };
  
  // Convert the ip address to the desired value
  // TODO: For numerical input

  // put the host’s address, and type into a socket structure
  memset ((char *) &server_addr, 0, sizeof server_addr);
  memcpy ((char *) &server_addr.sin_addr, hostinfo->h_addr, hostinfo->h_length);
  server_addr.sin_family= AF_INET;
  server_addr.sin_port= htons(port_number);
  // create a socket using TCP
  if ((sfd= socket(AF_INET, SOCK_STREAM, 0)) < 0)  {
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

//=============================================================================
// Main functions

void client_main(int argc, char * argv[])  {
  /* This is the main function for client */
  // TODO: Implement quit on the client

  // Initialization
	char						buffer[MSGSZ];
	char						msg[MSGSZ];
	char						ecid[MSGSZ];
  char            ip_name[30];
	char						packet_type[10];
	char						pipe_mux[10];
	char						s2c_fifo[10];
	char						src_self[10];
	char						src_server[7] = "server";
	char						word[MAXWORD];
  int             cid;
	int							sfd[2];
  int             ip_addr;
  int             port_number;
	int							switcher = 0;
	std::fstream		fp(argv[3]);
	std::string 		line;
	std::string			tokens[ARGAMT];

  // Last input client check
  if (argc != 6)  {
    // not enough arguments
    std::cout << "Wrong amount of arguments for client, consider following" << " running instructions" << std::endl;

    exit(EXIT_FAILURE);
  }
  
  cid = atoi(argv[1]);
  port_number = atoi(argv[5]);

	// Socket initialization
  sfd[0] = client_connect(argv[4], port_number);
	


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
	close(sfd[0]);

	// Unlink the client specific pipes
	unlink(s2c_fifo);
	unlink(csend->c2s_fifo);
	sleep(1);
	pthread_exit(NULL);
}

int main(int argc, char * argv[])  {
  // The main function
  user_inpt_err(argc, argv);

  if (argv[1][1] == 'c')  {
    // Running the client
    client_main(argc, argv);
  } else if (argv[1][1] == 's')  {
    // Running the server
  }
  return 0;
}