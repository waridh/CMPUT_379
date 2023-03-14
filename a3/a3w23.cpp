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
#define   CONTLINE      3
#define   MAXWORD       32
#define   MSGSZ         64
#define   NCLIENT       3
#define   NOBJECT       16
#define   PUTSZ         80
#define   SA            struct sockaddr
#define   TOKSZ         3

//=============================================================================
// Designing a struct
typedef struct  { double          msg[CONTLINE];}             MSG_DOUBLE;
typedef struct  { char            msg[CONTLINE][PUTSZ];}      MSG_CHAR;
typedef struct  { char             msg[MAXWORD];}             MSG_OBJ;
// Trying to use structs instead of sending bunch of strings.
typedef union   { MSG_DOUBLE m_d; MSG_CHAR m_c; MSG_OBJ m_i;} MSG;

typedef struct  { char kind[MAXWORD]; int lines; MSG msg;} FRAME;

#define   MAXBUF        sizeof(FRAME);




//=============================================================================
// Global variables

clock_t                                 start = times(NULL); // For gtime
int                                     list_count = 0; // Counting list
static long                             clktck = 0; // Clk work
std::map<int, std::string[CONTLINE]>    obj_list[NCLIENT];
// We are holding the objs

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
// Utilities 1
void send_2_items(int fd, char * msg1, char * msg2, char * src)  {
  // When we are sending two things to the file descriptor

  write(fd, msg1, sizeof(msg1));
  usleep(50);  // I needed to add this so that the process doesn't break socket
  write(fd, msg2, sizeof(msg2));

  std::cout << "Transmitted (src= " << src << ") "
  << "( " << msg1 << ", " << msg2 << ")" << std::endl;

  return;
}


//=============================================================================
// Commands
void delay_cmd(std::string delaytime)  {
  // Blocks the client for a certain time in milliseconds
  int         converted_time = stoi(delaytime);
  std::cout << std::endl;
	std::cout << "*** Entering a delay period of " << delaytime
			<< " msec" << std::endl;	
	usleep(converted_time*1000);
	std::cout << "*** Exiting delay period" << std::endl;
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

int put_cmd_client(std::fstream &fp, std::string & objname, int fd, char * cid)  {
  /* Since the object in put now has content, we need a function that can deal
  with it*/
  char            WSPACE[] = "\t ";
  char            * buffer;
  char            buffer2[MAXWORD + 1];
  char            buffer3[MAXWORD + 1];
  char            conbuff[PUTSZ + MAXWORD];
  char            sendingcont[CONTLINE][PUTSZ];
  FILE            * fp2 = fdopen(fd, "w");
  FRAME           send_packet;
  MSG             msg;
  int             i;
  int             linecount;
  std::string     oneline;

  std::cout << std::endl;
  memset( (char *) &send_packet, 0, sizeof(send_packet));
  memset( (char *) &msg, 0, sizeof(msg));
  // Establishing the frame
  write(fd, "PUT", sizeof("PUT"));
  sprintf(send_packet.kind, "PUT");
  while (std::getline(fp, oneline))  {
    // Grabbing the contents of the thing
    if (oneline == "}")  {
      break;
    }  else if (
      (oneline[0] == '#')
      || (oneline[0] == '\n')
      || (oneline.size() == 0)
      
    )  {
      // Ignoring the unimportant lines
      continue;
    }  else if (
      (oneline == "{")
    )  {
      linecount = 0;
      continue;
    }  else  {
      // The plan is to use strtok to get only the line.
      strcpy(conbuff, oneline.c_str());
      buffer = strtok(conbuff, WSPACE);
      if (buffer == objname + ':')  {
        // Checking if the name of the file is the same
        buffer = strtok(NULL, "\n");
        strcpy(msg.m_c.msg[linecount], buffer);
        linecount++;

        if (linecount > CONTLINE)  {
          // Error handling for when there are too many contents
          std::cout << "There are too many contents in object " << objname
          << std::endl;
          exit(EXIT_FAILURE);
        }
      }
    }
  }
  // Now we want to write the packet and message to the thing
  // strcpy(buffer2, objname.c_str());
  // sprintf(buffer2, "%s\n", objname.c_str());
  send_packet.msg = msg;
  if (write(fd, (char *) &send_packet, sizeof(send_packet)) < 0)  {
    // What the hey
    std::cout << "Failed to send the message" << std::endl;
  }
  std::cout << "We got past sending" << std::endl;
  std::cout << "Sent: " << buffer2 << std::endl;
  sprintf(buffer3, "%d", linecount);
  


  // send_2_items(fd, "PUT", buffer, cid);
  // for (i = 0; i < linecount; i++)  {
  //   // Thingy
  //   std::cout << sendingcont[i] << std::endl;
  // }
  return 0;
}

int put_cmd_server(int cid, int fd)  {
	/* This function adds an object to the list. Updated to match with the new
  requisites */
  char              buffer[MAXWORD + 1];
  FRAME             frame;

  memset( (char *) &frame, 0, sizeof(frame));

  // read(fd, buffer, sizeof(buffer));
  read(fd, (char *) &frame, sizeof(frame));
  std::cout << "This is buffer2: " << frame.msg.m_c.msg[0] << std::endl;

	// std::string				item_s = item;
	// int								cidi = atoi(cid);
	// if (obj_list[cidi].find(item_s) != obj_list[cidi].end())  {
	// 	// Already exists, return an error
	// 	return -1;
	// }  else  {
	// 	obj_list[cidi].insert(item_s);
	// 	return 0;
  // }
}

int get_cmd(char * cid, char * item)  {
	/* This function retrieves the object from the list*/
	// std::string				item_s = item;
	// for (auto i : obj_list)  {
	// 	/* Need to look at all the objects what was put here by all clients*/
	// 	if (i.second.find(item_s) != i.second.end())  {
	// 		/* If the object exists, then do normal return*/
	// 		return 0;
	// 	}
	// }
	/* The object does not exist in the thing, return error*/
	return -1;
}

int delete_cmd(char * cid, char * item)  {
	/* This function will delete the object that is stored*/
	// std::string				item_s = item;
	// int								cidi = atoi(cid);
	// if (obj_list[cidi].find(item_s) != obj_list[cidi].end())  {
	// 	/* If the object belongs to client, proceed as intended*/
	// 	obj_list[cidi].erase(item_s);
	// 	return 0;
	// }
	// for (auto i : obj_list)  {
	// 	/* Need to look at all the objects what was put here by all clients*/
	// 	if (i.first == cidi)  {
	// 		continue;
	// 	}  else if (i.second.find(item_s) != i.second.end())  {
	// 		/* If the object exists, but not owned by client*/
	// 		return -2;
	// 	}
	// }
	// /* The object does not exist in the thing*/
	// return -1;
}

//=============================================================================
// Utilities



void client_reciever(int fd, char * src)  {
  // Collects the response from the server
  char            buffer[MAXWORD];
  char            buffer2[MAXWORD];

  // Getting the first packet
  read(fd, buffer, sizeof(buffer));
  read(fd, buffer2, sizeof(buffer2));

  std::cout << std::endl;

  if (strncmp(buffer2, "UNLOVED", 7) == 0)  {
    // This is the message for no message
    std::cout << "Received (src= " << src << ") " << buffer << std::endl;
    return;
  }
  std::cout << "Received (src= " << src << ") " << "( " 
  << buffer << ": " << buffer2 << " )" << std::endl;
  // If the server wants the client to quit
  if (strncmp(buffer, "QUIT", 4) == 0)  {
    // The QUIT packet was sent back
    if (strncmp(buffer2, "ALCONN", 6) == 0)  {
      // The same client ID was already connected
      std::cout << "A client with this ID has already been connected"
      << std::endl;
      std::cout << "Please try again later" << std::endl;
      quit_cmd(fd);
    }  else if (strncmp(buffer2, "TOOLRG", 6) == 0)  {
      // This client ID is above the limit size
      std::cout << "This client ID is above the maximum allowed" << std::endl;
      std::cout << "Please try again later" << std::endl;
      quit_cmd(fd);
    }  else  {
      // I guess the server just wants the client to quit? Implementing it.
      quit_cmd(fd);
    }
  }

  return;
}

void server_receiver_print(int cid, char * packet, char * msg)  {
  // Outputting the received packet from the client

  std::cout << std::endl;
  if (strncmp(msg, "UNLOVED", 7) == 0)  {
    // For when the packet does not have a msg
    std::cout << "Received (src= client:" << cid << ") " << packet << std::endl;
    return;
  }

  std::cout << "Received (src= client:" << cid << ") "
  << "(" << packet << ": " << msg << ")" << std::endl;
  return;
}

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

void client_transmitter(int fd, std::string * tokens, std::fstream & fp)  {
  // Sending and recieving the thing without blocking
  char        buffer[MAXWORD];
  char        buffer2[MAXWORD];
  char        * fromwho = "server";
  FRAME       frame;
  MSG         msg;
  std::string contents[CONTLINE];

  // Clearing the memory of the thing
  memset( (char *) &frame, 0, sizeof(frame));
  memset( (char *) &msg, 0, sizeof(msg));

  if (tokens[1] == "delay")  {
    // First check if for delay
    delay_cmd(tokens[2]);
    return;
  }  else if (tokens[1] == "gtime")  {
    // Asking for the time since server began 
    strcpy(frame.kind, "GTIME");
    write(fd, (char *) &frame, sizeof(frame));
    // Using the gtime function, we send the packet
    client_reciever(fd, fromwho);
  }  else if (tokens[1] == "quit")  {
    // We want to send to the server so that it knows we want to appropriately
    // quit
    strcpy(buffer, "QUIT");
    write(fd, buffer, sizeof(buffer));
    client_reciever(fd, fromwho);
  }  else if (tokens[1] == "put")  {
    /* Handling the put command. Since we also need the name of the object, we
    will just send this into another function*/
    strcpy(buffer, tokens[0].c_str());
    put_cmd_client(fp, tokens[2], fd, buffer);

  }
}

void server_receiver(int cid, int fd, FRAME * frame)  {
  // This function will recieve the packets from the client
  char        buffer[MAXWORD];
  char        packet[MAXWORD];
  char        msgout[MAXWORD];
  char        * src = "server";
  double      gtimer;
  int         len;
  MSG         msg;

  // Clearing the struct

  if (strncmp(frame->kind, "GTIME", 5) == 0)  {
    // Checking for GTIME first as it is easier to implement
    // server_receiver_print(cid, inpacket, "UNLOVED");
    std::cout << "Just got GTIME" << std::endl;
    gtimer = gtime_cmd();  // Getting the time since program began
    // Making output packet and msg
    strcpy(packet, "TIME");
    sprintf(msgout, "%0.2f", gtimer);
    send_2_items(fd, packet, msgout, src);
  }  else if (strncmp(frame->kind, "PUT", 3) == 0)  {
    std::cout << "GOT INTO PUT" << std::endl;
    put_cmd_server(cid, fd);
  }

  return;
}


//=============================================================================
// Main functions

void client_sendcmds(int fd, int cid, char * filename)  {
  /* This command will read from the specified file and then send those to the
  fd specified. */
  char            buff0[MAXWORD];
  char            buffer[MAXWORD];
  char            filecont[CONTLINE][PUTSZ];
  std::fstream    fp(filename);
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
    strcpy(buffer, tokens[1].c_str());
    std::cout << cmdline << std::endl;
    client_transmitter(fd, tokens, fp);
  }

  return;
}

void client_main(int argc, char * argv[])  {
  /* This is the main function for client */

  // Initialization
  char            ip_name[30];
  char            buffer[MAXWORD];
  int             cid;
  int             fd;
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

  
  cid = atoi(argv[2]);
  port_number = atoi(argv[5]);

	// Socket initialization
  fd = client_connect(argv[4], port_number);

  // Confirming connections with the server 
  confirm_connection_client(fd, cid);

  // Need to loop and read the commands from the file and send it to the socket
  client_sendcmds(fd, cid, argv[3]);

	
	// Closing all the pipes from this end
	close(fd);

	// Unlink the client specific pipes
	sleep(1);
	exit(EXIT_SUCCESS);
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
  FRAME               frame;
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
            for (int i = 0; i < 2 + NCLIENT; i++) {
              // Closing all the file descriptors from this side
              close(pollfds[i].fd);
            }
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
          memset((char *) &frame, 0, sizeof(frame));
          len = read(pollfds[2 + i].fd, (char *) &frame, sizeof(frame));
          if (len == 0)  {
            // When we lose connection with the client
            std::cout << "lost connection to client " << connections[i]
            << std::endl;
            connections[i] = 0;
            continue;
          }  else if (strncmp(frame.kind, "QUIT", 4) == 0)  {
            // If the client sends the quit packet to the server
            
            write(pollfds[2 + i].fd, buffer, sizeof(buffer));
            strcpy(buffer, "UNLOVED");
            write(pollfds[2 + i].fd, buffer, sizeof(buffer));
            std::cout << std::endl;
            std::cout << "client " << connections[i] << " is disconnected"
            << std::endl;

            close(pollfds[2 + i].fd);

            connections[i] = 0;
            continue;
          }
          /* Receives and sends back the appropriate packet*/
          server_receiver(connections[i], pollfds[2 + i].fd, &frame);
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