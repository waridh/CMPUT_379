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

/* This substruct contains the doubles*/
typedef struct  { double          msg[CONTLINE];}             MSG_DOUBLE;
/* This substruct contains the character lines*/
typedef struct  { char            msg[CONTLINE][PUTSZ];}      MSG_CHAR;
// Trying to use structs instead of sending bunch of strings.
typedef union   { MSG_DOUBLE m_d; MSG_CHAR m_c;} MSG;
/* The full frame struct that contains the entire header and msg*/
typedef struct  {
  char  kind[MAXWORD];
  char  obj[MAXWORD];
  int   id;
  int   lines;
  MSG msg;
  } FRAME;

#define   MAXBUF        sizeof(FRAME);




//=============================================================================
// Global variables

clock_t                                 start = times(NULL); // For gtime
int                                     list_count = 0; // Counting list
static long                             clktck = 0; // Clk work
std::map<std::string, char[4][PUTSZ]>        obj_list[NCLIENT];
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
void receiver_print(FRAME * frame)  {
  // Outputting the received packet
  char          buffer[MAXWORD];
  int           i;
  int           lines;
  std::string   key;

  if (strncmp(frame->obj, "UNLOVED", 7) == 0)  {
    // For when the packet does not have a msg
    std::cout << "Received (src= " << frame->id << ") " << frame->kind << std::endl;
    return;
  }  else if (strncmp(frame->kind, "TIME", 4) == 0)  {
    // For outputting for TIME packet
    sprintf(buffer, "%0.2f", frame->msg.m_d.msg[0]);
  }  else  {
    // For most general cases
    sprintf(buffer, "%s", frame->obj);
  }

  std::cout << "Received (src= " << frame->id << ") "
  << "(" << frame->kind << ":\t" << buffer << ")" << std::endl;

  if (strncmp(frame->kind, "PUT", 3) == 0)  {
    // Need to print out the content of the put packet
    key = frame->obj;
    
    for (i = 0; i < frame->lines; i++)  {
      // Outputting the content that was received
      std::cout << " [" << i << "]: '"
      << frame->msg.m_c.msg[i] << "'" << std::endl;
    }
  }
  return;
}

void transmitter_print(FRAME * frame)  {
  // Outputting the received packet
  char        buffer[MAXWORD];
  int         i;

  if (strncmp(frame->obj, "UNLOVED", 7) == 0)  {
    // For when the packet does not have a msg
    std::cout << "Transmitted (src= " << frame->id << ") " << frame->kind << std::endl;
    return;
  }  else if (strncmp(frame->kind, "TIME", 4) == 0)  {
    // Checking for time
    sprintf(buffer, "%0.2f", frame->msg.m_d.msg[0]);
  }  else  {
    // For most general cases
    sprintf(buffer, "%s", frame->obj);
  }

  std::cout << "Transmitted (src= " << frame->id << ") "
  << "(" << frame->kind << ":\t" << buffer << ")" << std::endl;
  if  (strncmp(frame->kind, "PUT", 3) == 0)  {
    // Special case for put
    for (i = 0; i < frame->lines; i++)  {
      // Looping the content out
      std::cout << " [" << i << "]: '" << frame->msg.m_c.msg[i]
      << "'" << std::endl;
    }
  }
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

int put_cmd_client(std::fstream &fp, std::string & objname, int fd, FRAME * frame)  {
  /* Since the object in put now has content, we need a function that can deal
  with it*/
  char            WSPACE[] = "\t ";
  char            * buffer;
  char            buffer2[MAXWORD + 1];
  char            buffer3[MAXWORD + 1];
  char            conbuff[PUTSZ + MAXWORD];
  char            sendingcont[CONTLINE][PUTSZ];
  FILE            * fp2 = fdopen(fd, "w");
  FRAME           inframe;
  MSG             msg;
  int             i;
  int             linecount;
  std::string     oneline;

  std::cout << std::endl;
  memset( (char *) &msg, 0, sizeof(msg));
  // Establishing the frame
  sprintf(frame->kind, "PUT");
  strcpy(frame->obj, objname.c_str());
  while (std::getline(fp, oneline))  {
    // Grabbing the contents of the thing
    if (oneline == "}")  {
      frame->lines = linecount;
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
      sprintf(buffer2, "%s:", objname.c_str());
      if (strncmp(buffer, buffer2, sizeof(buffer2)) == 0)  {
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
  frame->msg = msg;
  if (write(fd, (char *) frame, sizeof(*frame)) < 0)  {
    // What the hey
    std::cout << "Failed to send the message" << std::endl;
  }
  transmitter_print(frame);

  // Need to wait for server confirmation
  if (read(fd, (char *) &inframe, sizeof(inframe)) < 1)  {
    // Error check
    std::cout << "Failed to receive the message from server" << std::endl;
  }

  receiver_print(&inframe);
  
  return 0;
}

int put_cmd_server(int fd, FRAME * frame)  {
	/* This function adds an object to the list. Updated to match with the new
  requisites */
  char              buffer[MAXWORD + 1];
  int               i;
  std::string       key(frame->obj);
  /*
  TODO: Find out why the hell we are saying that it exists
  */
  // Unpacking the packet
  
  if  (
    obj_list[frame->id - 1].find(key)
    != obj_list[frame->id - 1].end()
    )  {
      std::cout << frame->obj << std::endl;
      std::cout << obj_list[frame->id - 1][frame->obj] << std::endl;
    // Checking if the key exists in the map
    std::cout << std::endl;
    receiver_print(frame);
    // Need to create the error packet
    memset((char *) frame, 0, sizeof(*frame));
    frame->id = 0;
    strcpy(frame->kind, "ERROR");
    strcpy(frame->obj, "object already exists");
    write(fd, (char *) frame, sizeof(*frame));
    transmitter_print(frame);

    return -1;
  }

  sprintf(obj_list[frame->id - 1][frame->obj][3], "%d", frame->lines);
  for (i = 0; i < frame->lines; i++)  {

    strcpy(obj_list[frame->id - 1][frame->obj][i], frame->msg.m_c.msg[i]);
  }

  // Print out what was received
  std::cout << std::endl;
  receiver_print(frame);

  // Sending back ok packet
  memset((char *) frame, 0, sizeof(*frame));
  frame->id = 0;
  strcpy(frame->kind, "OK");
  strcpy(frame->obj, "UNLOVED");
  write(fd, (char *) frame, sizeof(*frame));
  return 0;

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




void client_reciever(int fd)  {
  // Collects the response from the server
  char            buffer[MAXWORD];
  FRAME           inframe;

  // Getting the first packet
  // read(fd, buffer, sizeof(buffer));
  // read(fd, buffer2, sizeof(buffer2));
  read(fd, (char *) &inframe, sizeof(inframe));

  receiver_print(&inframe);

  // If the server wants the client to quit
  if (strncmp(inframe.kind, "QUIT", 4) == 0)  {
    // The QUIT packet was sent back
    if (strncmp(inframe.obj, "ALCONN", 6) == 0)  {
      // The same client ID was already connected
      std::cout << "A client with this ID has already been connected"
      << std::endl;
      std::cout << "Please try again later" << std::endl;
      quit_cmd(fd);
    }  else if (strncmp(inframe.obj, "TOOLRG", 6) == 0)  {
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

void client_transmitter(int fd, std::string * tokens, std::fstream & fp, int src)  {
  // Sending and recieving the thing without blocking
  char        buffer[MAXWORD];
  char        buffer2[MAXWORD];
  FRAME       frame;
  MSG         msg;
  std::string contents[CONTLINE];

  // Clearing the memory of the thing
  memset( (char *) &frame, 0, sizeof(frame));
  memset( (char *) &msg, 0, sizeof(msg));

  frame.id = src;

  if (tokens[1] == "delay")  {
    // First check if for delay
    delay_cmd(tokens[2]);
    return;
  }  else if (tokens[1] == "gtime")  {
    // Asking for the time since server began
    // Making the output packet 
    strcpy(frame.kind, "GTIME");
    strcpy(frame.obj, "UNLOVED");
    // Sending the output packet
    write(fd, (char *) &frame, sizeof(frame));
    std::cout << std::endl;
    transmitter_print(&frame);  // Printing to terminal what we sent

    client_reciever(fd);  // Waiting for server response
  }  else if (tokens[1] == "quit")  {
    // We want to send to the server so that it knows we want to appropriately
    // quit
    strcpy(frame.kind, "QUIT");
    strcpy(frame.obj, "UNLOVED");
    write(fd, (char *) &frame, sizeof(frame));
    client_reciever(fd);
  }  else if (tokens[1] == "put")  {
    /* Handling the put command. Since we also need the name of the object, we
    will just send this into another function*/
  
    put_cmd_client(fp, tokens[2], fd, &frame);

  }
}

void server_receiver(int cid, int fd, FRAME * frame)  {
  // This function will recieve the packets from the client
  char        buffer[MAXWORD];
  char        packet[MAXWORD];
  char        msgout[MAXWORD];
  double      gtimer;
  int         len;
  int         src = 0;
  FRAME       frameout;
  MSG         msg;

  // Clearing the struct

  if (strncmp(frame->kind, "GTIME", 5) == 0)  {
    // Checking for GTIME first as it is easier to implement
    // server_receiver_print(cid, inpacket, "UNLOVED");

    // Clearing the struct being sent back
    memset((char *) &frameout, 0, sizeof(frameout));
    memset((char *) &msg, 0, sizeof(msg));
    std::cout << std::endl;
    receiver_print(frame);
    // Making output packet and msg
    strcpy(frameout.kind, "TIME");
    frameout.id = src;
    frameout.msg.m_d.msg[0] = gtime_cmd(); // Time since program began
    // Writing the time packet back
    write(fd, (char *) &frameout, sizeof(frameout));

    transmitter_print(&frameout);

  }  else if (strncmp(frame->kind, "PUT", 3) == 0)  {
    put_cmd_server(fd, frame);
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
    client_transmitter(fd, tokens, fp, cid);
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
  /*
  TODO: Figure out what is wrong with the socket, cause the stuff is losing
        connections
  */
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