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
#include <signal.h>
#include <string>
#include <sys/times.h>
#include <unistd.h>
#include <unordered_map>

// Define

// Global variables
// Many of these become read-only after initial declaration in main
static clock_t                start = times(NULL);
static struct tms             cpustart;
static int                    DEBUG = 1;
static uint                   NITER;
static int                    RESOURCET_COUNT = 0;
static long                   clktck = 0;  // For conversion into seconds
static RESOURCES_T            RESOURCE_MAP;
static AVAILR_T               AVAILR_MAP;

// Synchronization

static pthread_barrier_t      bar1;  // Barrier so all threads start at once
static pthread_barrier_t      bar2;  // Barrier for stopping all the threads
static pthread_mutex_t        mutex1;  // Not sure what for
static pthread_mutex_t        monitormutex;
static pthread_mutex_t        outputlock;  // So that the threads can take turn
static pthread_mutex_t        clock_setter;  // A critical section access

// Critical sections



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
        std::cout << "The monitorTime field is not an integer. "
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

void barrier_err()  {
  std::cout << "Failed to initialize the barrier object." << std::endl
  << "Quitting" << std::endl;
  exit(EXIT_FAILURE);
}

//=============================================================================
// Utilities

double time_since_start(clock_t * startTime)  {
  clock_t               current_time = times(NULL);
  clock_t               c_seconds_passed = current_time - (*startTime);
  double                mseconds_passed;

  // Setting up the clktck
  pthread_mutex_lock(&clock_setter);
  if  (clktck == 0)  {
    clktck = sysconf(_SC_CLK_TCK);
    if (clktck < 0)  {
      // Error handling
      std::cout << "sysconf error" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  pthread_mutex_unlock(&clock_setter);

  // The actual finding of seconds. (It's actually in ms)
  mseconds_passed = (c_seconds_passed / (double) clktck) * 1000.0;
  return mseconds_passed;
}

void timeprint(
	clock_t &time1,
	clock_t &time2,
	struct tms &cpu1,
	struct tms &cpu2)  {
	/* This function will try to print to format of figure 8.31. We are using the
  testbook: Advanced Programming in the UNIX Environment third Edition*/
	pthread_mutex_lock(&clock_setter);
	if (clktck == 0)  {  // If we don't already have the tick
		clktck=sysconf(_SC_CLK_TCK);
		if (clktck < 0)  {
			std::cout << "sysconf error" << std::endl;  // Error handling from apue
			exit(EXIT_FAILURE);
		}
	}
  pthread_mutex_lock(&clock_setter);

	// Most of the code is gathered from the APUE textbook here
  clock_t real = time2 - time1;
  printf(" real: %7.2f sec.\n", real / (double) clktck);
  printf(" user: %7.2f sec.\n",
  (cpu2.tms_utime - cpu1.tms_utime) / (double) clktck);
  printf(" sys: %7.2f sec.\n",
  (cpu2.tms_stime - cpu1.tms_stime) / (double) clktck);
  
	return;
}

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

int cmdline_eater(int argc, char * argv[], int * monitorTime)  {
  // Collects the data from the command line arguments
  char *                  dynamicBuff;
  int                     tokenscount;
  int                     taskcount = 0;
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
    }  else if (tokens[0] == "task")  {
      // We want to count the amount of threads that will be created
      taskcount++;
    }
  }
  std::cout << std::endl;
  std::cout << "Resources given:" << std::endl;
  for  (auto i : RESOURCE_MAP.resources)  {
    // Printing out useful resource informations
    std::cout << '\t' << i.first << ": " << i.second << std::endl;
  }

  // Setting up the index of each resource.
  delete[] dynamicBuff;
  fp.close();
  return taskcount;
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
// Signals

void monitor_signal(int signum)  {
  pthread_mutex_lock(&outputlock);
  std::cout << std::endl << "All task threads are done" << std::endl;
  std::cout << "Exiting monitor thread" << std::endl;
  pthread_mutex_unlock(&outputlock);
  pthread_mutex_unlock(&monitormutex);
  pthread_exit(NULL);
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

void thread_creator(
  std::string *           task_line,
  int                     idx,
  int                     tokenscount,
  THREADREQUIREMENTS *    inputstruct,
  pthread_t *             thread
  )  {
  int             i;
  int             requiredr;
  std::string     rname;

  // Allocating the required information into the struct being passed into tid
  inputstruct->name = task_line[1].c_str();
  inputstruct->busyTime = stoi(task_line[2]);
  inputstruct->idleTime = stoi(task_line[3]);
  inputstruct->rtypes = tokenscount-4;
  inputstruct->idx = idx;
   
  for  (i = 4; i < tokenscount; i++)  {
    // Collecting the resource requirements in storing it in the struct
    // TODO: Error handling for when the resource listed wasn't allocated
    /* TODO: Need to allocate an array of input structs for the threads*/
    
    requiredr = colon_tokenize(&(task_line[i]), &rname);
    if (
      inputstruct->requiredr.find(rname)
      != inputstruct->requiredr.end())  {
      /* Handling for when the input file is weird and shows the same resource
      multiple times*/
      // Checking if the key already exists
      inputstruct->requiredr[rname] += requiredr;
    }  else  {
      inputstruct->requiredr[rname] = requiredr;
    }
  }
  // Creating the thread now
  pthread_create(
    thread,
    NULL,
    task_thread,
    (void *) inputstruct
  );
  return;
}

void thread_creation(
  char *                  filename,
  THREADREQUIREMENTS *    threadr,
  pthread_t *             threads
  )  {
  char *                  dynamicBuff;
  uint                    resourceidx = 0;
  uint                    i;
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
      
      thread_creator(
        tokens,
        resourceidx,
        tokenscount,
        threadr + resourceidx,
        threads + resourceidx);
      resourceidx++;  // So that we can allocate the data required by the thread
    }
  }

  // Debugging output for the threads that have allocated
  if (DEBUG)  {
    std::cout << std::endl;
    std::cout << "Task threads:\n\t";
    for (i = 0; i < resourceidx; i++)  {
      std::cout << "Name: " << threadr[i].name << ", idx: "
      << threadr[i].idx << ", busyTime: " << threadr[i].busyTime
      << ", idleTime: " << threadr[i].idleTime << ", ResourcesTypes: "
      << threadr[i].rtypes << ", ";
      for (auto j : threadr[i].requiredr)  {
        std::cout << j.first << ": " << j.second << ", ";
      }
      if  (i != resourceidx - 1)  {
        std::cout << "\n\t";
      }  else  {
        std::cout << std::endl;
      }
    }
  }
  
  delete[] dynamicBuff;
  fp.close();
  return;
}

void  thread_main(
  char * filename,
  int tasksamount,
  pthread_t * monitorthread
  )  {
  int                     i;
  THREADREQUIREMENTS *    threadresources;
  pthread_t *             threads;

  // Allocation of memory
  threadresources = new THREADREQUIREMENTS[tasksamount];  // Dynamic allocation
  threads = new pthread_t[tasksamount];  // Dynamically allocating the threads

  // Synchronication initialization
  if (pthread_barrier_init(&bar1, NULL, tasksamount + 1) != 0)  {
    // Error handling
    barrier_err();
  }  else if (pthread_barrier_init(&bar2, NULL, tasksamount + 1) != 0)  {
    barrier_err();
  }

  // Task thread creation function
  thread_creation(filename, threadresources, threads);

  // Synchronization of exit
  pthread_barrier_wait(&bar2);
  pthread_kill(*monitorthread, SIGALRM);
  pthread_mutex_lock(&monitormutex);  // monitor sync
  // Join the thread and freeing the resources
  std::cout << std::endl << "Freeing resources: " << std::endl;
  for (i = 0; i < tasksamount; i++)  {
    // Waiting to collect the threads
    pthread_join(threads[i], NULL);
    std::cout << "\tCollected thread at index: " << i << std::endl;
  }
  pthread_mutex_unlock(&monitormutex);
  // Deleting the dynamically allocated stuff from the heap
  delete[] threadresources;
  delete[] threads;

  // Other destruction methods
  pthread_barrier_destroy(&bar1);
  pthread_barrier_destroy(&bar2);
  return;
}

//=============================================================================
// Thread functions

void * monitor_thread(void * arg)  {
  // Creates the output every monitorTime millisecs
  int *         monitorTime = (int *) arg;
  int           test = 0;

  // Setting up signal handler for when the main thread wants to kill this one
  signal(SIGALRM, monitor_signal);

  // Sync for the exit of the program
  pthread_mutex_lock(&monitormutex);
  // Barier implement so that the monitor starts at the same time as the thread
  pthread_barrier_wait(&bar1);

  // Implement thread synchronization so that we don't get unexpected thing

  while (1)  {
    pthread_mutex_lock(&outputlock);
    std::cout << std::endl << "montor: " << test << std::endl;
    pthread_mutex_unlock(&outputlock);
    usleep((*monitorTime) * 1000);
    test++;
  }
  return NULL;
}

void * task_thread(void * arg)  {
  /* Simulates the task required by the assignment*/
  uint                    completed = 0;
  THREADREQUIREMENTS *    info = (THREADREQUIREMENTS *) arg;

  // Starting up synchronization
  pthread_barrier_wait(&bar1);

  pthread_mutex_lock(&outputlock);
  std::cout << std::endl;
  std::cout << "Starting task " << info->name << std::endl;
  pthread_mutex_unlock(&outputlock);

  while (completed < NITER)  {
    /* The main task thread loop*/
    completed++;
  }

  // Synchronization of thread exit
  pthread_barrier_wait(&bar2);

  pthread_exit(NULL);
}

int  main(int argc, char * argv[])  {
  // Running the main function
  int         monitorTime;
  int         taskcount;
  pthread_t   montid;
  
  cmdline_err(argc, argv);
  taskcount = cmdline_eater(argc, argv, &monitorTime);  /* Process some command
  line args */
  
  // Making the monitor thread
  pthread_create(
    &montid,
    NULL,
    monitor_thread,
    (void *) &monitorTime
  );
  pthread_detach(montid);

  thread_main(argv[1], taskcount, &montid);

  return 0;
}