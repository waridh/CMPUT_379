// a4w23.cpp
/*
The final assignment for CMPUT 379
  name: Waridh 'Bach' Wongwandanee
  ccid: waridh
  sid:  1603722
*/

/* Because this is an assignment, I don't want to split the program into
multiple files. That seems more annoying for submission. I will try using a
header file though.*/

// Libraries
#include "a4w23.h"
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <unordered_map>

// Define

#define MAXWORD         32
#define NRES_TYPES      10
#define NTASKS          25

// Global variables

int                 NITER;
int                 RESOURCET_COUNT = 0;
RESOURCES_T         RESOURCE_MAP;
AVAILR_T            AVAILR_MAP;

// Critical sections

int                 something;

//=============================================================================
// Error handling

void  cmdline_err(int argc, char * argv[])  {
  // Checks if the user used the command line arguments properly
  size_t        i;
  if (argc != 4)  {
    // Not the correct amount of command line arguments
    std::cout << std::endl;
    std::cout << "Incorrect amount of command line arguments" << std::endl;
    std::cout << "Quitting" << std::endl;
    exit(EXIT_FAILURE);
  }  else  {
    // Checking if the last and second last argument is an integer.
    for (i = 0; i < strlen(argv[2]); i++)  {
      // Checking through each character of the last argument to see if it digit
      if (isdigit(argv[2][i]))  {
        // Don't do anything
        continue;
      }  else  {
        std::cout << std::endl;
        std::cout << "The monitorTIme field is not an integer. "
        << "Please use the proper command." << std::endl << "Quitting"
        << std::endl;
        exit(EXIT_FAILURE);
      }
    }

    for (i = 0; i < strlen(argv[3]); i++)  {
      // Checking through each character of the last argument to see if it digit
      if (isdigit(argv[3][i]))  {
        // Don't do anything
        continue;
      }  else  {
        std::cout << std::endl;
        std::cout << "The last field is not an integer. Please use the proper "
        << "command." << std::endl << "Quitting" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
}

//=============================================================================
// Utilities

int tokenizer(char * cmdline, std::string * tokens)  {
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
  
  return count;  // So we know how many tokens there are
}

void cmdline_eater(int argc, char * argv[], int * monitorTime)  {
  // Collects the data from the command line arguments
  char *                  dynamicBuff;
  int                     tokenscount;
  std::fstream            fp(argv[1]);
  std::string             readline;
  std::string             tokens[NRES_TYPES + 4];

  NITER = atoi(argv[3]);  // Getting NITER
  *monitorTime = atoi(argv[2]);  // Collecting monitor time

  while (std::getline(fp, readline))  {
    // Getting the line inputs and storing the resources
    if  (
      (readline[0] == '#')
      || (readline[0] == '\n')
      || (readline.size() == 0)
    )  {
      // Ignoring the comments and newlines
      continue;
    }

    // Resizing the cstring
    dynamicBuff = new char[readline.size()];
    strcpy(dynamicBuff, readline.c_str());
    tokenscount = tokenizer(dynamicBuff, tokens);
    // Tokenize and then get the first word
    
    if  (tokens[0] == "resources")  {
      /* When it's a resource thing */
      resource_gatherer(tokens, tokenscount);
    }
  }

  std::cout << "Resources given:\n\t";
  for  (auto i : RESOURCE_MAP.resources)  {
    // Printing out useful resource informations
    std::cout << i.first << ": " << i.second << " ";
  }
  std::cout << std::endl;
  delete[] dynamicBuff;
  fp.close();
  return;
}

int colon_tokenize(std::string * pair, std::string * name)  {
  char *          dynamicbuffer = new char[pair->size()];
  int             value;

  strcpy(dynamicbuffer, pair->c_str());

  *name = strtok(dynamicbuffer, ":");
  value = atoi(strtok(NULL, ""));

  delete[] dynamicbuffer;  // I'm a profession, I dynamically allocate memory
  return value;
}


//=============================================================================
// String parser

void resource_gatherer(std::string * resource_line, int tokenscount)  {
  /* We want to read the resources from the file*/
  int             i;
  int             maxamount;
  std::string     name;
  for  (i = 1; i < tokenscount; i++)  {
    // Need to separate the name and the number based on the column
    maxamount = colon_tokenize(&(resource_line[i]), &name);
    // Mass insertion into the map

    if (RESOURCE_MAP.resources.find(name) != RESOURCE_MAP.resources.end())  {
      // Checking if the key already exists
      RESOURCE_MAP.resources[name] += maxamount;
    }  else  {
      RESOURCE_MAP.resources[name] = maxamount;
      RESOURCET_COUNT++;
    }
  }
  return;
}

void thread_creator(std::string * task_line, int tokenscount)  {
  int             i;
  int             requiredr;
  std::string     rname;
  

  return;
}

void thread_creation(char * filename)  {
  char *                  dynamicBuff;
  int                     tokenscount;
  std::fstream            fp(filename);
  std::string             readline;
  std::string             tokens[NRES_TYPES + 4];

  while (std::getline(fp, readline))  {
    // Getting the line inputs and storing the resources
    if  (
      (readline[0] == '#')
      || (readline[0] == '\n')
      || (readline.size() == 0)
    )  {
      // Ignoring the comments and newlines
      continue;
    }

    // Resizing the cstring
    dynamicBuff = new char[readline.size()];
    strcpy(dynamicBuff, readline.c_str());
    tokenscount = tokenizer(dynamicBuff, tokens);
    // Tokenize and then get the first word
    
    if  (tokens[0] == "task")  {
      /* When it's a resource thing */
      
    }
  }

  std::cout << "Resources given:\n\t";
  for  (auto i : RESOURCE_MAP.resources)  {
    // Printing out useful resource informations
    std::cout << i.first << ": " << i.second << " ";
  }
  std::cout << std::endl;
  delete[] dynamicBuff;
  fp.close();
  return;
}



//=============================================================================
// Thread functions

void * monitor_thread(void * arg)  {
  // Creates the output every monitorTime millisecs
  int *         monitorTime = (int *) arg;
  int           test = 0;

  // Implement thread synchronization so that we don't get unexpected thing

  while (1)  {
    std::cout << std::endl << "montor: " << test << std::endl;
    usleep((*monitorTime) * 1000);
    test++;
  }
  return NULL;
}

void * task_thread(void * arg)  {
  /* Simulates the task required by the assignment*/
  

  pthread_exit(NULL);
}

int  main(int argc, char * argv[])  {
  // Running the main function
  int         monitorTime;
  pthread_t   montid;
  
  cmdline_err(argc, argv);
  cmdline_eater(argc, argv, &monitorTime);  // Process some command line args
  
  // Making the monitor thread
  pthread_create(
    &montid,
    NULL,
    monitor_thread,
    (void *) &monitorTime
  );
  pthread_detach(montid);
  sleep(5);
  return 0;
}