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
#include <iomanip>
#include <iostream>
#include <map>
#include <pthread.h>
#include <signal.h>
#include <string>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unordered_map>

// Define

// Global variables
// Many of these become read-only after initial declaration in main
static clock_t                start = times(NULL);
static int                    DEBUG = 1;
static uint                   NITER;
static int                    RESOURCET_COUNT = 0;
static long                   clktck = 0;  // For conversion into seconds
static RESOURCES_T            RESOURCE_MAP;

// Synchronization

static pthread_barrier_t      bar1;  // Barrier so all threads start at once
static pthread_barrier_t      bar2;  // Barrier for stopping all the threads
static pthread_barrier_t      bar3;  // Need this one to let threads start good
static pthread_barrier_t      bar4;  // For the final closing thread output
static pthread_mutex_t        monitormutex;  // Mutex for the monitor thread
static pthread_mutex_t        outputlock;  // So that the threads can take turn
static pthread_mutex_t        clock_setter;  // A critical section access
static pthread_mutex_t        resourceaccess;  // For checking the thing
static pthread_mutex_t        monitoraccess;  // Getting running details

// Critical sections

static MONITOR_T              taskmanager;  // Details on what is running
static AVAILR_T               AVAILR_MAP;

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
  // Statically allocating some stuff because it's on the stack and has low mem
  /* Funny error: For some reason the dynamically allocated code is breaking.
  I don't know why, we we are making a kinda big buffer on the stack for this.*/
  char *                  staticBuff;
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

    // Dynamically allocating memory;
    staticBuff = new char[readline.size()];
    strcpy(staticBuff, readline.c_str());
    tokenscount = tokenizer(staticBuff, tokens);
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
    AVAILR_MAP.resources[i.first] = i.second;
  }

  delete[] staticBuff;
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
  if  (pthread_create(
    thread,
    NULL,
    task_thread,
    (void *) inputstruct
  ) != 0)  {
    std::cout << "Failed to create thread: " << inputstruct->name << std::endl;
    exit(EXIT_FAILURE);  // Error handling
  };
  return;
}

void thread_creation(
  char *                  filename,
  THREADREQUIREMENTS *    threadr,
  pthread_t *             threads
  )  {

  /* We were using dynamic buffer at one point, but turns out, it wanted to
  die. I am guessing that we will never get to 1024 char size per line*/
  char *                  staticBuff;  // Misnomer, it's actually dynamic
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
    staticBuff = new char[readline.size()];
    strcpy(staticBuff, readline.c_str());
  
    tokenscount = tokenizer(staticBuff, tokens);
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
    pthread_mutex_lock(&outputlock);
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
    pthread_mutex_unlock(&outputlock);
  }
  // Pre-output for starting thread
  pthread_mutex_lock(&outputlock);
  std::cout << std::endl;
  std::cout << "Running threads:" << std::endl;
  pthread_mutex_unlock(&outputlock);

  delete[] staticBuff;  // Releasing memory
  fp.close();  // Closing input stream
  return;
}

void  thread_main(
  char * filename,
  int tasksamount,
  pthread_t * monitorthread
  )  {
  int                       i;
  THREADREQUIREMENTS *      threadresources;
  pthread_t *               threads;

  // Allocation of memory
  threadresources = new THREADREQUIREMENTS[tasksamount];
  threads = new pthread_t[tasksamount];

  // Synchronication initialization
  if (pthread_barrier_init(&bar1, NULL, tasksamount + 1) != 0)  {
    barrier_err();  // Error handling
  }  else if (pthread_barrier_init(&bar2, NULL, tasksamount + 1) != 0)  {
    barrier_err();  // Error handling
  }  else if (pthread_barrier_init(&bar3, NULL, tasksamount + 1) != 0)  {
    barrier_err();
  }  else if (pthread_barrier_init(&bar4, NULL, tasksamount + 1))  {
    barrier_err();  // Error handling
  }

  // Task thread creation function
  thread_creation(filename, threadresources, threads);

  pthread_barrier_wait(&bar3);  // Synchonizing the start

  pthread_barrier_wait(&bar2);  // Synchronizing the exit
  pthread_kill(*monitorthread, SIGALRM);  // Ending the monitor thread
  pthread_mutex_lock(&monitormutex);  // monitor sync
  // Join the thread and freeing the resources
  std::cout << std::endl << "Freeing resources: " << std::endl;
  pthread_barrier_wait(&bar4);
  for  (i = 0; i < tasksamount; i++)  {
    // Joining the thread
    pthread_join(threads[i], NULL);
  }
  pthread_mutex_unlock(&monitormutex);

  // Deleting the dynamically allocated stuff from the heap
  delete[] threadresources;
  delete[] threads;

  // Other destruction methods
  pthread_barrier_destroy(&bar1);
  pthread_barrier_destroy(&bar2);
  pthread_barrier_destroy(&bar3);
  pthread_barrier_destroy(&bar4);
  return;
}

//=============================================================================
// Thread functions

void * monitor_thread(void * arg)  {
  // Creates the output every monitorTime millisecs
  int *         monitorTime = (int *) arg;

  // Setting up signal handler for when the main thread wants to kill this one
  signal(SIGALRM, monitor_signal);

  pthread_mutex_lock(&monitoraccess);
  taskmanager.idlecount = 0;
  taskmanager.runcount = 0;
  taskmanager.waitcount = 0;
  taskmanager.donecount = 0;
  pthread_mutex_unlock(&monitoraccess);

  // Sync for the exit of the program
  pthread_mutex_lock(&monitormutex);
  // Barier implement so that the monitor starts at the same time as the thread
  pthread_barrier_wait(&bar1);

  // Implement thread synchronization so that we don't get unexpected thing

  while (1)  {

    // Critical section safety rails
    pthread_mutex_lock(&outputlock);
    pthread_mutex_lock(&monitoraccess);

    // Printing out the waiting threads
    std::cout << std::endl << "montor:\t[WAIT]\t";
    for  (auto i : taskmanager.wait)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    // Printing out the running threads
    std::cout << "\t[RUN]\t";
    for  (auto i : taskmanager.run)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    // Printing out the idle threads
    std::cout << "\t[IDLE]\t";
    for  (auto i : taskmanager.idle)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    std::cout << "\t[DONE]\t";
    for  (auto i : taskmanager.done)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    std::cout << std::endl;

    pthread_mutex_unlock(&monitoraccess);
    pthread_mutex_unlock(&outputlock);
    usleep((*monitorTime) * 1000);
  }
  return NULL;
}

void * task_thread(void * arg)  {
  /* Simulates the task required by the assignment*/
  clock_t                 tstart;
  int                     ran = 1;
  int                     j;
  uint                    completed = 0;
  pthread_t               tid = pthread_self();
  THREADREQUIREMENTS *    info = (THREADREQUIREMENTS *) arg;

  // Error handling for when the thread needs more than available
  // Since truly read only, I am not going to protect access
  for  (auto i : info->requiredr)  {
    if  (RESOURCE_MAP.resources.count(i.first) == 0)  {
      // When the thread is requesting a non-existent resource type
      pthread_mutex_lock(&outputlock);
      std::cout << "Thread: " << info->name << " (tid= " << std::hex << tid
      << ") is requesting a resource that wasn't allocated ("
      << i.first << ")" << std::endl
      << "Quitting" << std::endl;
      std::cout.copyfmt(std::ios(NULL));
      exit(EXIT_FAILURE);
      pthread_mutex_unlock(&outputlock);
    }
    else if  (i.second > RESOURCE_MAP.resources[i.first])  {
      // The thread is requesting more resources than available in the system
      pthread_mutex_lock(&outputlock);
      std::cout << "Thread: " << info->name << " (tid= " << std::hex << tid
      << ") is requesting more resources than available" << std::endl
      << "Quitting" << std::endl;
      std::cout.copyfmt(std::ios(NULL));
      exit(EXIT_FAILURE);
      pthread_mutex_unlock(&outputlock);
    }
  }

  // Start up synchronization
  pthread_barrier_wait(&bar3);

  // Starting up allocation
  tstart = times(NULL);
  pthread_mutex_lock(&outputlock);
  
  std::cout << "\tStarting task: " << info->name << "\t(tid= " << std::hex
  << tid << ")" << std::endl;
  std::cout.copyfmt(std::ios(NULL));
  pthread_mutex_unlock(&outputlock);

  pthread_mutex_lock(&monitoraccess);
  taskmanager.idle.insert(info->name);
  taskmanager.idlecount++;
  pthread_mutex_unlock(&monitoraccess);

  pthread_barrier_wait(&bar1);

  while (completed < NITER)  {
    /* The main task thread loop*/

    // Put the thread into waiting
    pthread_mutex_lock(&monitoraccess);
    taskmanager.idle.erase(info->name);
    taskmanager.idlecount--;
    taskmanager.wait.insert(info-> name);
    taskmanager.waitcount++; 
    pthread_mutex_unlock(&monitoraccess);

    // Going through loop to access the resources
    for  (auto i : info->requiredr)  {
      // Iterating through ordered map. Since ordered, it will not cycle
      for  (j = 0; j < i.second; j++)  {
        // Truly checking resources one by one. Should prevent deadlock this way
        pthread_mutex_lock(&AVAILR_MAP.emptylock[i.first]);
        pthread_mutex_lock(&resourceaccess);  // Needed for resource return
        // Taking the resource
        AVAILR_MAP.resources[i.first]--;

        pthread_mutex_unlock(&resourceaccess);
        if  (AVAILR_MAP.resources[i.first] != 0)  {
          pthread_mutex_unlock(&AVAILR_MAP.emptylock[i.first]);
        }
        
      }
    }
    if (ran)  {
      // Taking the busy time to run. We have gotten all the resource we needed
      pthread_mutex_lock(&monitoraccess);
      taskmanager.wait.erase(info->name);
      taskmanager.waitcount--;
      taskmanager.run.insert(info->name);
      taskmanager.runcount++;
      pthread_mutex_unlock(&monitoraccess);
      usleep((info->busyTime) * 1000);

      // Sending command line output
      pthread_mutex_lock(&outputlock);
      std::cout << "task: " << info->name << " (tid= 0x" << std::hex << tid;
      std::cout.copyfmt(std::ios(NULL));
      std::cout << ", iter=" << completed << ", time= "
      << time_since_start(&tstart) << " msec)" << std::endl;
      pthread_mutex_unlock(&outputlock);
      
      // Releasing resources
      // Going through loop to access the resources
      for  (auto i : info->requiredr)  {
        // Iterating through ordered map. Since ordered, it will not cycle
        for  (j = 0; j < i.second; j++)  {
          // Truly checking resources one by one. Should prevent deadlock this way
          pthread_mutex_lock(&resourceaccess);
          if  (AVAILR_MAP.resources[i.first] == 0)  {
            // For when we are returning resource to an empty struct
            AVAILR_MAP.resources[i.first]++;
            pthread_mutex_unlock(&AVAILR_MAP.emptylock[i.first]);
          }  else  {
            AVAILR_MAP.resources[i.first]++;
          }
          pthread_mutex_unlock(&resourceaccess);
          
        }
      }

      // Putting the thread into idle
      pthread_mutex_lock(&monitoraccess);
      taskmanager.run.erase(info->name);
      taskmanager.runcount--;
      taskmanager.idle.insert(info->name);
      taskmanager.idlecount++;
      pthread_mutex_unlock(&monitoraccess);
      usleep((info->idleTime) * 1000);  // When the thread is idling

      completed++;
    }
  }

  // Putting the thread into the DONE category
  pthread_mutex_lock(&monitoraccess);  // Entering monitor CS
  taskmanager.idle.erase(info->name);
  taskmanager.idlecount--;
  taskmanager.done.insert(info->name);
  taskmanager.donecount++;
  pthread_mutex_unlock(&monitoraccess);

  pthread_barrier_wait(&bar2);  // Notifying main thread that we are done
  pthread_barrier_wait(&bar4);  // Synchronizing for exit output
  pthread_mutex_lock(&outputlock);
  std::cout << "\tFreeing thread: " << info->name << "\t(tid= "
  << std::hex << tid << ")" << std::endl;
  pthread_mutex_unlock(&outputlock);

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