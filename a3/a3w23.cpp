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
typedef union   { MSG_DOUBLE m_d; MSG_CHAR m_c;}              MSG;
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
struct tms                              cpu1;
clock_t                                 start = times(&cpu1); // For gtime
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

void timeprint(
	clock_t &time1,
	clock_t &time2,
	struct tms &cpu1,
	struct tms &cpu2)  {
	/* This function will try to print to format of figure 8.31. We are using the
testbook: Advanced Programming in the UNIX Environment third Edition*/
	static long clktck = 0;
	if (clktck == 0)  {  // If we don't already have the tick
		clktck=sysconf(_SC_CLK_TCK);
		if (clktck < 0)  {
			std::cout << "sysconf error" << std::endl;  // Error handling from apue
			exit(EXIT_FAILURE);
		}
	}
	// Most of the code is gathered from the APUE textbook here
  clock_t real = time2 - time1;
  printf(" real: %7.2f sec.\n", real / (double) clktck);
  printf(" user: %7.2f sec.\n",
  (cpu2.tms_utime - cpu1.tms_utime) / (double) clktck);
  printf(" sys: %7.2f sec.\n",
  (cpu2.tms_stime - cpu1.tms_stime) / (double) clktck);
  
	return;
}


void receiver_print(FRAME * frame)  {
  // Outputting the received packet
  char          buffer[MAXWORD];
  int           i;
  std::string   key;

  if (
    (strncmp(frame->obj, "UNLOVED", 7) == 0)
    || (strncmp(frame->kind, "OK", 2) == 0))  {
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

  if (strncmp(frame->kind, "HELLO", 5) == 0)  {
    // When we send the HELLO packet
    std::cout << "Transmitted (src= " << frame->id << ") (HELLO, idNumber= "
    << frame->id << ")" << std::endl;
    return;
  }
  if (
    (strncmp(frame->obj, "UNLOVED", 7) == 0)
    || (strncmp(frame->kind, "OK", 2) == 0)
    )  {
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
  struct tms        cpu2;
  clock_t           end;


	std::cout << "do_client: client closing connection" << std::endl << std::endl;
  end = times(&cpu2);
  timeprint(start, end, cpu1, cpu2);
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

int put_cmd_client(
  std::fstream &fp,
  std::string & objname,
  int fd,
  FRAME * frame)  {
  /* Since the object in put now has content, we need a function that can deal
  with it*/
  char            WSPACE[] = "\t ";
  char            * buffer;
  char            buffer2[MAXWORD + 1];
  char            conbuff[PUTSZ + MAXWORD];
  FRAME           inframe;
  MSG             msg;
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
      /*
      TODO:
      So in the commented out code, we were actually able to isolate the
      content without the stuff. Actually, just check at the lab tomorrow.
      */
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
  int               i;
  std::string       key(frame->obj);
  // Unpacking the packet
  
  if  (obj_list[frame->id - 1].find(key) != obj_list[frame->id - 1].end())  {
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
  transmitter_print(frame);
  return 0;

}

int get_cmd_client(int fd, FRAME * frame, std::string & objname)  {
	/*
  The get command from the client. Sends the requesting packet, and then
  wait for the response
  */
  int             i;
	strcpy(frame->kind, "GET");
  strcpy(frame->obj, objname.c_str());
  if (write(fd, (char *) frame, sizeof(* frame)) <= 0)  {
    // Error handling
    std::cout << "*** Message failed to send" << std::endl;
    std::cout << "Quitting" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << std::endl;
  transmitter_print(frame);

  // I am too lazy to allocate memory to have another frame, so we are reusing
  // the frame

  memset((char *) frame, 0, sizeof(*frame));
  if (read(fd, (char *) frame, sizeof(* frame)) <= 0)  {
    // Error handling for reading a weird length msg
    std::cout << "*** Failed to receive the message from the server"
    << std::endl;
    std::cout << "Quitting" << std::endl;
    exit(EXIT_FAILURE);
  }
  // Output what we received
  receiver_print(frame);
  for (i = 0; i < frame->lines; i++)  {
    // Printing out the content
    std::cout << " [" << i << "]: '" << frame->msg.m_c.msg[i]
    << "'" << std::endl;
  }


  return 0;
}

int get_cmd_server(int fd, FRAME * frame)  {
  /*
  This function handles the get cmd on the server side
  */
  int           i;
  int           j;
  FRAME         outframe;
  MSG           msg;
  std::string   key(frame->obj);

  // Initializing the packet
  memset((char *) &outframe, 0, sizeof(outframe));
  memset((char *) &msg, 0, sizeof(msg));
  outframe.id = 0;

  // Printing the receiver
  std::cout << std::endl;
  receiver_print(frame);

  // Pre-allocate the response for when the object just doesn't exist in the
  // server
  strcpy(outframe.kind, "ERROR"); sprintf(outframe.obj, "object not found");
  outframe.lines = 1; strcpy(outframe.msg.m_c.msg[0], "object not found");

  // Doing a check for if the file exists
  for  (i = 0; i < NCLIENT; i++)  {
    if  (obj_list[i].find(key) != obj_list[i].end())  {
      /* Found the object we are looking for, send back ok packet with the
      content in the msg*/
      
      strcpy(outframe.kind, "OK");
      sprintf(outframe.obj, "@@@GIFT");

      outframe.lines = atoi(obj_list[i][key][NCLIENT]);
      for  (j = 0; j < outframe.lines; j++)  {
        // Copying the content into the packet
        strcpy(msg.m_c.msg[j], obj_list[i][key][j]);
      }
      outframe.msg = msg;
      break;
    }
  }

  write(fd, (char *) &outframe, sizeof(outframe));
  transmitter_print(&outframe);
  return 0;

}

int delete_cmd_server(int fd, FRAME * frame)  {
  /*
  The delete packet should get rid of the content stored in the dictionary
  and if it doesn't exist, then we should return an error packet
  */
  int           i;
  FRAME         outframe;
  std::string   key(frame->obj);

  // Initializing the packet
  memset((char *) &outframe, 0, sizeof(outframe));
  outframe.id = 0;

  // Printing the receiver
  std::cout << std::endl;
  receiver_print(frame);

  // Pre-allocate the response for when the object just doesn't exist in the
  // server
  strcpy(outframe.kind, "ERROR"); sprintf(outframe.obj, "object not found");

  // Doing a check for if the file exists
  for (i = 0; i < NCLIENT; i++)  {
    if  (obj_list[i].find(key) != obj_list[i].end())  {
    // Check if the object exists in the server or not
      if (i == frame->id - 1)  {
        // In this case, the object exists and it is owned by the client
        obj_list[i].erase(key);
        strcpy(outframe.kind, "OK"); strcpy(outframe.obj, "UNLOVED");
        break;
      }  else  {
        // Means that we found the object that doesn't belong to the client
        strcpy(outframe.kind, "ERROR");
        sprintf(outframe.obj, "client not owner");
      }
    }
  }

  write(fd, (char *) &outframe, sizeof(outframe));
  transmitter_print(&outframe);
  return 0;
}

int delete_cmd_client(int fd, FRAME * frame, std::string & objname)  {
  /*
  This is the client side of the delete command. Simply should just send a
  packet, and then wait to receive the response from the server.
  */
  FRAME           inframe;
  std::string     oneline;

  std::cout << std::endl;

  // Establishing the frame
  sprintf(frame->kind, "DELETE");
  strcpy(frame->obj, objname.c_str());

  if (write(fd, (char *) frame, sizeof(*frame)) < 0)  {
    // What the hey
    std::cout << "Failed to send the message" << std::endl;
  }
  transmitter_print(frame);

  // Setting the inframe
  memset((char *) &inframe, 0, sizeof(inframe));
  // Need to wait for server confirmation
  if (read(fd, (char *) &inframe, sizeof(inframe)) < 1)  {
    // Error check
    std::cout << "Failed to receive the message from server" << std::endl;
  }
  receiver_print(&inframe);
  
  return 0;
}

//=============================================================================
// Utilities

void client_reciever(int fd)  {
  // Collects the response from the server
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
  }  else if  (
    (strncmp(inframe.kind, "OK", 2) == 0)
    && (strncmp(inframe.obj, "@@@ILOVEYOU", 12) == 0))  {
    // For proper exits
    quit_cmd(fd);
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

  // Program start message
  std::cout << std::endl;
  std::cout << "a3w23: do_server" << std::endl;
  std::cout << "Server is accepting connections (port= " << port_number
  << ")" << std::endl;
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
  int             len;
  FRAME           starterframe;

  memset((char *) &starterframe, 0, sizeof(starterframe));
  // Setting up HELLO packet
  starterframe.id = cid;
  strcpy(starterframe.kind, "HELLO");
  len = write(fd, (char *) &starterframe, sizeof(starterframe));
  if (len <= 0)  {
    std::cout << "Couldn't write HELLO packet. Quitting" << std::endl;
    exit(EXIT_FAILURE);
  }
  std::cout << std::endl;
  transmitter_print(&starterframe);
  // Waiting for confirmation from server
  memset((char *) &starterframe, 0, sizeof(starterframe));
  len = read(fd, &starterframe, sizeof(starterframe));
  
  receiver_print(&starterframe);
  return 0;
}

int confirm_connection_server(int fd)  {
  // The server making sure the client sends a HELLO packet. Returns the CID
  int             cid;
  int             len;
  FRAME           instarter;

  // Setting the incoming frame up
  memset((char *) &instarter, 0, sizeof(instarter));


  // Reading buffer and then checking if it is appropriate
  len = read(fd, (char *) &instarter, sizeof(instarter));

  if (len <= 0)  {
    // Error handling
    std::cout << "The server failed to read the HELLO packet. Disconnecting"
    << std::endl;
    close(fd);
    return -1;
  }
  if (strncmp(instarter.kind, "HELLO", 5) == 0)  {
    // Checking packet
    cid = instarter.id;
    std::cout << std::endl;
    std::cout << "Recieved (src= " << cid << ") (HELLO, idNumber= "
    << instarter.id << ")" << std::endl;
    
    // Prepping to send back the OK packet
    memset((char *) &instarter, 0, sizeof(instarter));
    instarter.id = 0;
    strcpy(instarter.kind, "OK");
    strcpy(instarter.obj, "UNLOVED");
    write(fd, (char *) & instarter, sizeof(instarter));
    transmitter_print(&instarter);
    return cid;
  }  else  {
    // Did not get the HELLO packet
    std::cout << "Did not recieve the HELLO packet, disconnecting" << std::endl;
    close(fd);
    return -1;
  }

}

void client_transmitter(int fd, std::string * tokens, std::fstream & fp, int src)  {
  // Sending and recieving the thing without blocking
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
    strcpy(frame.kind, "DONE");
    strcpy(frame.obj, "UNLOVED");
    write(fd, (char *) &frame, sizeof(frame));
    // Notifying the client that we have sent the DONE packet
    std::cout << std::endl;
    transmitter_print(&frame);
    client_reciever(fd);
  }  else if (tokens[1] == "put")  {
    /* Handling the put command. Since we also need the name of the object, we
    will just send this into another function*/
  
    put_cmd_client(fp, tokens[2], fd, &frame);

  }  else if (tokens[1] == "delete")  {
    /* Handler for delete.*/
    delete_cmd_client(fd, &frame, tokens[2]);
  }  else if (tokens[1] == "get")  {
    /* Sending the get packet*/
    get_cmd_client(fd, &frame, tokens[2]);
  }
}

void server_receiver(int cid, int fd, FRAME * frame)  {
  // This function will recieve the packets from the client
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
    // This is the handler for the PUT packet
    put_cmd_server(fd, frame);
  }  else if (strncmp(frame->kind, "DELETE", 6) == 0)  {
    // For the delete command we run the handler for it
    delete_cmd_server(fd, frame);
  }  else if (strncmp(frame->kind, "GET", 3) == 0)  {
    // For the get command
    get_cmd_server(fd, frame);
  }

  return;
}

void list_print()  {
  /* Outputs the contents of the stored objects*/
  int         i, k, lines;
  int         checker = 0;

  std::cout << std::endl << "Stored object table:" << std::endl;

  // Loop through the different clients and output the stored data
  for  (i = 0; i < NCLIENT; i++)  {
    // Now loop through the map
    for  (auto j : obj_list[i])  {
      // Output
      std::cout << "(owner= " << i + 1 << ", name= " << j.first << ")"
      << std::endl;
      lines = atoi(j.second[3]);
      for (k = 0; k < lines; k++)  {
        std::cout << "[" << k << "] '" << j.second[k] << "'" << std::endl;
      }
      checker++;
    }
  }
  if (checker == 0)  {
    // Need to create output for when there isn't an item saved
    std::cout << "no objects saved" << std::endl;
  }
}

//=============================================================================
// Main functions

void client_sendcmds(int fd, int cid, char * filename)  {
  /* This command will read from the specified file and then send those to the
  fd specified. */
  char            buff0[MAXWORD];
  char            buffer[MAXWORD];
  std::fstream    fp(filename);
  std::string     cmdline;
  std::string     tokens[TOKSZ];


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
    client_transmitter(fd, tokens, fp, cid);
  }

  return;
}

void client_main(int argc, char * argv[])  {
  /* This is the main function for client */

  // Initialization
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

  std::cout << std::endl;

  std::cout << "main: do_client (idNumber= " << cid << ", inputFile= '"
  << argv[3] << "')" << std::endl;

  std::cout << "\t" << "(server= '" << argv[4] << "', port= " << port_number
  << ")" << std::endl;
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
  clock_t             endtime;
  int                 buffd;
  int                 cid;
  int                 connected_clients = 0;
  int                 connections[NCLIENT] = {0};
  int                 i;
  int                 len;
  int                 port_number;
  int                 timeout = 100;
  FRAME               frame;
  struct pollfd       pollfds[2 + NCLIENT];
  struct sockaddr_in  frominfo;
  struct tms          cpu2;
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
            std::cout << std::endl;
						std::cout << "quitting" << std::endl;
            std::cout << "do_server: server closing main socket (";
            for (int i = 0; i < 2 + NCLIENT; i++) {
              // Closing all the file descriptors from this side
              // TODO: Ask what the heck is this
              std::cout << "done[" << i+1 << "]= " << close(pollfds[1].fd) + 1
              << ", ";
            }
            std::cout << ")" << std::endl << std::endl;
            endtime = times(&cpu2);
            timeprint(start, endtime, cpu1, cpu2);
            exit(EXIT_SUCCESS);
						
					}  else if (strncmp(buffer, "list", 4) == 0)  {
						list_print();
						
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
            std::cout << std::endl
            << "rcvFrame: received frame has zero length." << std::endl;
            std::cout << "server lost connection to client " << connections[i]
            << std::endl;
            connections[i] = 0;
            continue;
          }  else if (strncmp(frame.kind, "DONE", 4) == 0)  {
            // If the client sends the quit packet to the server
            std::cout << std::endl;
            receiver_print(&frame);
            frame.id = 0;
            strcpy(frame.kind, "OK");
            strcpy(frame.obj, "@@@ILOVEYOU");
            write(pollfds[2 + i].fd, (char *) &frame, sizeof(frame));
            transmitter_print(&frame);

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