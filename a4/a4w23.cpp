// a4w23.cpp
/*
The final assignment for CMPUT 379
  name: Waridh 'Bach' Wongwandanee
  ccid: waridh
  sid:  1603722
*/

/* Because this is an assignment, I don't want to split the program into
multiple files. That seems more annoying for submission. I will try using a
header file though.

Extra note: Most of the function descriptions are in the header file
CLANG++
*/

// Libraries
#include "a4w23.h"
#include <assert.h>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <string>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <unordered_map>

// Global variables
// Many of these become read-only after initial declaration in main
static clock_t                start = times(NULL);
static const int              DEBUG = 0;  // Debug flag when stuff is breaking
static uint                   NITER;  // Number of iterations the tasks need
static int                    RESOURCET_COUNT = 0;
static long                   clktck = 0;  // For conversion into seconds
static RESOURCES_T            RESOURCE_MAP;  // Max amount of resources

// Synchronization

static pthread_barrier_t      bar1;  // Barrier so all threads start at once
static pthread_barrier_t      bar2;  // Barrier for stopping all the threads
static pthread_barrier_t      bar3;  // Need this one to let threads start good
static pthread_barrier_t      bar4;  // For the final closing thread output
static pthread_barrier_t      bar5;  // Another barrier
static pthread_barrier_t      bar6;  // Monitor thread end synchronization

static pthread_cond_t         resourcecond;  // Using simplified method

static pthread_mutex_t        outputlock;  // So that the threads can take turn
static pthread_mutex_t        clock_setter;  // A critical section access
static pthread_mutex_t        resourceaccess;  // For checking the thing
static pthread_mutex_t        monitoraccess;  // Getting running details

// Critical sections

static MONITOR_T              taskmanager;  // Details on what is running
static AVAILR_T               AVAILR_MAP;
static int                    exitidx;  // For syncing the output

// Shared object functions

void barrier_ender()  {

  // Unallocating the barriers
  pthread_barrier_destroy(&bar1);
  pthread_barrier_destroy(&bar2);
  pthread_barrier_destroy(&bar3);
  pthread_barrier_destroy(&bar4);
  pthread_barrier_destroy(&bar5);
  pthread_barrier_destroy(&bar6);
}

void barrier_creator(int tasksamount)  {
  // Synchronication initialization
  if (pthread_barrier_init(&bar1, NULL, tasksamount + 1) != 0)  {
    barrier_err();  // Error handling
  }  else if (pthread_barrier_init(&bar2, NULL, tasksamount + 1) != 0)  {
    barrier_err();  // Error handling
  }  else if (pthread_barrier_init(&bar3, NULL, tasksamount + 1) != 0)  {
    barrier_err();
  }  else if (pthread_barrier_init(&bar4, NULL, tasksamount + 1) != 0)  {
    barrier_err();  // Error handling
  }  else if (pthread_barrier_init(&bar5, NULL, tasksamount + 1) != 0)  {
    barrier_err();
  }  else if (pthread_barrier_init(&bar6, NULL, 2) != 0)  {
    barrier_err();
  }
}

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

void thread_init_err(THREADREQUIREMENTS * info, pthread_t * tid)  {
  // Going to treat it as a critical section
  pthread_mutex_lock(&resourceaccess);  // Guard in case there is some write
  for  (auto i : info->requiredr)  {
    if  (RESOURCE_MAP.resources.count(i.first) == 0)  {
      // When the thread is requesting a non-existent resource type
      pthread_mutex_lock(&outputlock);
      std::cout << "Thread: " << info->name << " (tid= " << std::hex << *tid
      << ") is requesting a resource that wasn't allocated ("
      << i.first << ")" << std::endl
      << "Quitting" << std::endl;
      std::cout.copyfmt(std::ios(NULL));
      exit(EXIT_FAILURE);
      pthread_mutex_unlock(&outputlock);
    }  else if  (i.second > RESOURCE_MAP.resources[i.first])  {
      // The thread is requesting more resources than available in the system
      pthread_mutex_lock(&outputlock);
      std::cout << "Thread: " << info->name << " (tid= " << std::hex << *tid
      << ") is requesting more resources than available" << std::endl
      << "Quitting" << std::endl;
      std::cout.copyfmt(std::ios(NULL));
      exit(EXIT_FAILURE);
      pthread_mutex_unlock(&outputlock);
    }
  }
  pthread_mutex_unlock(&resourceaccess);
  return;
}

//=============================================================================
// Utilities

unsigned long long time_since_start(clock_t * startTime)  {
  clock_t               current_time = times(NULL);
  clock_t               c_seconds_passed = current_time - (*startTime);
  unsigned long long    mseconds_passed;

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
  mseconds_passed = (unsigned long long)
  ((c_seconds_passed / (long double) clktck) * 1000.0);
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

void resourceprint()  {
  pthread_mutex_lock(&outputlock);
  pthread_mutex_lock(&resourceaccess);
  std::cout << "\nSystem Resources:\n";
  for  (auto i : RESOURCE_MAP.resources)  {
    // Printing out all the resources
    std::cout << "\t" << i.first << ":\t(maxAvail=\t" << i.second << ", held=\t"
    << (i.second - AVAILR_MAP.resources[i.first]) << ")" << std::endl;
  }  // Did an actual check if there were held resources
  pthread_mutex_unlock(&resourceaccess);
  pthread_mutex_unlock(&outputlock);
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
  char                    staticBuff[1024];  // Dynamically allocated array suck
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

  for  (auto i : RESOURCE_MAP.resources)  {
    // Allocating the available resources
    AVAILR_MAP.resources[i.first] = i.second;
  }
  if (DEBUG)  {
    // A check on how many resources we have at the beginning
    resourceprint();
    // std::cout << std::endl;
    // std::cout << "Resources given:" << std::endl;
  }
  

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
  pthread_mutex_trylock(&monitoraccess);
  pthread_mutex_lock(&outputlock);
  // Sending out the final monitor output
  if  (DEBUG)  {
    // Sending output for what resources are available
    pthread_mutex_lock(&resourceaccess);
    std::cout << std::endl << "resources: ";
    for  (auto i : AVAILR_MAP.resources)  {
      std::cout << '\t' << i.first << ": " << i.second << std::endl;
    }
    pthread_mutex_unlock(&resourceaccess);
  }

  // Printing out the waiting threads
  std::cout << std::endl << "monitor:\t[WAIT]\t";
  for  (auto i : taskmanager.wait)  {
    std::cout << i << ' ';
  }
  std::cout << std::endl;

  // Printing out the running threads
  std::cout << "\t\t[RUN]\t";
  for  (auto i : taskmanager.run)  {
    std::cout << i << ' ';
  }
  std::cout << std::endl;

  // Printing out the idle threads
  std::cout << "\t\t[IDLE]\t";
  for  (auto i : taskmanager.idle)  {
    std::cout << i << ' ';
  }
  std::cout << std::endl;
  pthread_mutex_unlock(&outputlock);  // Letting go of the output lock

  resourceprint();  // Printing out the resource allocated to system
  pthread_mutex_unlock(&monitoraccess);
  pthread_barrier_wait(&bar6);  // Synchronizing with the main thread

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

    if (RESOURCE_MAP.resources.count(name) != 0)  {
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
  THREADMAP *             threadm
  )  {
  int             i;
  int             requiredr;
  std::string     rname;

  

  // Handling for duplicate task names
  if  ((threadm->thread.count(task_line[1])) != 0 )  {
    // To this program, this is an error in the input file. End the program
    std::cout << "The task\t[" << task_line[1] << "] is a duplicate, please"
    << " fix the input file\nQuitting" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Allocating the required information into map
  threadm->thread[task_line[1].c_str()].info.name = task_line[1].c_str();
  try  {  // Error handling for stoi
    threadm->thread[task_line[1].c_str()].info.busyTime = stoi(task_line[2]);
  }  catch(std::exception& e)  {
    // stoi failed
    std::cout << "Error converting the busy time to integer for task["
    << task_line[1] << "]\nQuitting" << std::endl;
    exit(EXIT_FAILURE);
  }
  try  {  // More error handling for stoi
    threadm->thread[task_line[1].c_str()].info.idleTime = stoi(task_line[3]);
  }  catch(std::exception& e)  {
    std::cout << "Error converting the idle time to integer for task["
    << task_line[1] << "]\nQuitting" << std::endl;
  }
  threadm->thread[task_line[1].c_str()].info.rtypes = tokenscount-4;
  threadm->thread[task_line[1].c_str()].info.idx = idx;
   
  for  (i = 4; i < tokenscount; i++)  {
    // Collecting the resource requirements in storing it in the struct
    
    requiredr = colon_tokenize(&(task_line[i]), &rname);
    
    if  (threadm->thread[task_line[1]].info.requiredr.count(rname) != 0)  {
      threadm->thread[task_line[1]].info.requiredr[rname] += requiredr;
    }  else  {
      threadm->thread[task_line[1]].info.requiredr[rname] = requiredr;
    }
  }

  // Creating the thread now
  if  (pthread_create(
    &(threadm->thread[task_line[1]].tid),
    NULL,
    task_thread,
    (void *) &(threadm->thread[task_line[1]].info)
  ) != 0)  {  // Error handling for failed thread creation
    std::cout << "Failed to create thread: "
    << threadm->thread[task_line[1]].info.name << std::endl;
    exit(EXIT_FAILURE);  // Error handling
  };

  pthread_detach(threadm->thread[task_line[1]].tid);
  return;
}

void thread_creation(
  char *                  filename,
  THREADMAP *             thread_map
  )  {

  /* We were using dynamic buffer at one point, but turns out, it wanted to
  die. I am guessing that we will never get to 1024 char size per line*/
  char                    staticBuff[1024];
  uint                    resourceidx = 0;
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
  
    strcpy(staticBuff, readline.c_str());
  
    tokenscount = tokenizer(staticBuff, tokens);
    // Tokenize and then get the first word
    
    if  (tokens[0] == "task")  {
      /* When it's a resource thing */
      
      thread_creator(
        tokens,
        resourceidx,
        tokenscount, 
        thread_map
      );
      resourceidx++;  // So that we can allocate the data required by the thread
    }
  }
  
  // Debugging output for the threads that have allocated
  pthread_mutex_lock(&outputlock);
  if  (DEBUG)  {
    std::cout << std::endl;
    std::cout << "Task threads:\n\t";
    for  (auto  i : thread_map->thread)  {
      std::cout << "Name: " << i.first << ", busyTime: " << i.second.info.busyTime
      << ", idleTime: " << i.second.info.idleTime << ", ResourcesTypes: "
      << i.second.info.rtypes << ", ";
      for (auto j : i.second.info.requiredr)  {
        std::cout << j.first << ": " << j.second << ", ";
      }
      std::cout << "\n\t";
    }
  }
  std::cout << std::endl; 
  std::cout << "Running threads:" << std::endl;

  pthread_mutex_unlock(&outputlock);

  fp.close();  // Closing input stream
  return;
}

void  thread_main(
  char * filename,
  int tasksamount,
  pthread_t * monitorthread
  )  {
  THREADMAP                 thread_map; 

  // Synchronication initialization
  barrier_creator(tasksamount); 

  // Task thread creation function
  thread_creation(filename, &thread_map);

  pthread_barrier_wait(&bar3);  // Synchonizing the start

  pthread_barrier_wait(&bar2);  // Synchronizing the exit

  pthread_kill(*monitorthread, SIGALRM);  // Ending the monitor thread
  
  pthread_barrier_wait(&bar6);  // Waiting for the monitor to close

  // Join the thread and freeing the resources
  pthread_mutex_lock(&outputlock);
  std::cout << std::endl << "System Tasks: " << std::endl;
  pthread_mutex_unlock(&outputlock);

  pthread_barrier_wait(&bar4);  // Synchronication for letting the thread output

  pthread_barrier_wait(&bar5);  // Knowing that the threads have all completed

  barrier_ender();

  
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
  pthread_mutex_unlock(&monitoraccess);

  // Barier implement so that the monitor starts at the same time as the thread
  pthread_barrier_wait(&bar1);

  // Implement thread synchronization so that we don't get unexpected thing

  while (1)  {
    
    // Debugging check for how many resources are available
    pthread_mutex_lock(&outputlock);
    if  (DEBUG)  {
      // Sending output for what resources are available
      pthread_mutex_unlock(&outputlock);
      resourceprint();
      pthread_mutex_lock(&outputlock);
    }

    // Critical section safety rails 
    pthread_mutex_lock(&monitoraccess);

    // Printing out the waiting threads
    std::cout << std::endl << "monitor:\t[WAIT]\t";
    for  (auto i : taskmanager.wait)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    // Printing out the running threads
    std::cout << "\t\t[RUN]\t";
    for  (auto i : taskmanager.run)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl;

    // Printing out the idle threads
    std::cout << "\t\t[IDLE]\t";
    for  (auto i : taskmanager.idle)  {
      std::cout << i << ' ';
    }
    std::cout << std::endl; 

    std::cout << std::endl;

    pthread_mutex_unlock(&monitoraccess);
    pthread_mutex_unlock(&outputlock);

    usleep((*monitorTime) * 1000);  // Rest interval in milliseconds
  }
  return NULL;
}

void * task_thread(void * arg)  {
  /* Simulates the task required by the assignment*/
  clock_t                               waitStart;
  unsigned long long                    waitTime = 0;
  int                                   ran;
  uint                                  completed = 0;
  pthread_t                             tid = pthread_self();
  std::unordered_map<std::string, int>  holding;
  THREADREQUIREMENTS *                  info = (THREADREQUIREMENTS *) arg;

  // Error handling for when the thread needs more than available
  thread_init_err(info, &tid);

  // Start up synchronization
  pthread_barrier_wait(&bar3);  // We like to keep the threads at around same

  // Starting up allocation
  
  pthread_mutex_lock(&outputlock);
  // Starting task text is a race condition, however, I don't think it matters
  std::cout << "\tStarting task: " << info->name << "\t(tid= " << std::hex
  << tid << ")" << std::endl;
  std::cout.copyfmt(std::ios(NULL));
  pthread_mutex_unlock(&outputlock);

  // Initializing the resource held
  for  (auto i : info->requiredr)  {
    holding[i.first] = 0;
  }

  pthread_mutex_lock(&monitoraccess);  // Preplacing the task into the idle
  taskmanager.idle.insert(info->name);
  taskmanager.idlecount++;
  pthread_mutex_unlock(&monitoraccess);
 
  pthread_barrier_wait(&bar1);  // Synchronizing with the other threads
  while (completed < NITER)  {
    /* The main task thread loop*/

    // Put the thread into waiting
    pthread_mutex_lock(&monitoraccess);
    taskmanager.idle.erase(info->name);
    taskmanager.idlecount--;
    taskmanager.wait.insert(info-> name);
    taskmanager.waitcount++; 
    pthread_mutex_unlock(&monitoraccess);

    waitStart = times(NULL);  // Setting up for wait for accurate reading

    /* Checking if there are enough resources to run the program*/
    pthread_mutex_lock(&resourceaccess);

    // Going through loop to access the resources
    while  (1)  {
      // While loop for condition checking
      ran = 1;
      for  (auto i : info->requiredr)  {
        // Checks for resources
        if (AVAILR_MAP.resources[i.first] < i.second)  {
          // Setting a flag that there wasn't enough
          ran = 0;
          break;
        }
      }
      if (!ran)  {
        // There wasn't enough resources
        /* Now we wait for the task taking the resource to finish and send a
        condition signal before rechecking the stuff*/
        pthread_cond_wait(
          &resourcecond,
          &resourceaccess
        );
        continue;
      }  else  {
        // Passing the check for having enough resources
        for  (auto i : info->requiredr)  {
          AVAILR_MAP.resources[i.first] -= i.second;  // Actually taking it
          holding[i.first] += i.second;  // Adding to the holding map
        }
        break;
      }
    }
    pthread_mutex_unlock(&resourceaccess);  // Done setting up for the program
    // Keeping track of how long it's been waiting
    waitTime += time_since_start(&waitStart);

    if (ran)  {
      /* We get into this condition when there are enough resources available */

      // Taking the busy time to run. We have gotten all the resource we needed
      pthread_mutex_lock(&monitoraccess);  // Updating the monitor structure
      taskmanager.wait.erase(info->name);
      taskmanager.waitcount--;
      taskmanager.run.insert(info->name);
      taskmanager.runcount++;
      pthread_mutex_unlock(&monitoraccess);
      usleep((info->busyTime) * 1000);  // Spending time holding resources
 
      /* Releasing the resources*/
      // Going through loop to access the resources
      pthread_mutex_lock(&resourceaccess);
      for  (auto i : info->requiredr)  {
        AVAILR_MAP.resources[i.first] += i.second;
        holding[i.first] -= i.second;
        assert(
          AVAILR_MAP.resources[i.first] <= RESOURCE_MAP.resources[i.first]
        );
        assert(holding[i.first] >= 0);
      }
      pthread_cond_signal(&resourcecond);  // Wake up the waiting thread
      pthread_mutex_unlock(&resourceaccess);  // Let those threads fight for it 

      // Putting the thread into idle
      pthread_mutex_lock(&monitoraccess);
      taskmanager.run.erase(info->name);
      taskmanager.runcount--;
      taskmanager.idle.insert(info->name);
      taskmanager.idlecount++;
      pthread_mutex_unlock(&monitoraccess);
      usleep((info->idleTime) * 1000);  // When the thread is idling

      // Sending command line output (We have completed an iteration)
      pthread_mutex_lock(&outputlock);  // Locking for output
      std::cout << "task: " << info->name << " (tid= 0x" << std::hex << tid;
      std::cout.copyfmt(std::ios(NULL));  // Turning off hexadecimal
      std::cout << ", iter=" << completed << ", time= "
      << time_since_start(&start) << " msec)" << std::endl;
      pthread_mutex_unlock(&outputlock);

      completed++;
    }  else  {  // Error handling for a really strange case
      pthread_mutex_lock(&outputlock);
      std::cout << "It should have been impossible to get past the blocking "
      << "without having all the resources\nQuitting" << std::endl;
      pthread_mutex_unlock(&outputlock);
      resourceprint();
      exit(EXIT_FAILURE);
    }
  }

  pthread_barrier_wait(&bar2);  // Notifying main thread that we are done
  pthread_barrier_wait(&bar4);  // Synchronizing for exit output

  // Final information output. Need to be output in order
 
  while  (1)  {
    /* Check so that we output the final output in order. Technically this is
    less safe and more work than just outputting this information on the main
    thread, where all this data is store, but I already implemented it and it
    is relavent*/
    pthread_mutex_lock(&resourceaccess);
    if  (info->idx == exitidx)  {
      // This is the correct index that needs to be output
      exitidx++;
      break;
    }
    pthread_mutex_unlock(&resourceaccess);
  }
  pthread_mutex_lock(&outputlock);  // Creating the freed thread output
  std::cout << "[" << info->idx << "] " << info->name << " (IDLE, runTime= "
  << info->busyTime << " msec, idleTime= " << info->idleTime
  << " msec):\n\t(tid= " << std::hex << tid << ")" << std::endl;
  std::cout.copyfmt(std::ios(NULL));
  for  (auto i : info->requiredr)  {
    // Printing out the required resources
    std::cout << "\t" << i.first << ":\t(needed=\t" << i.second << ", held=\t" 
    << holding[i.first] << ")" << std::endl;
  }
  std::cout << "\t(RUN: " << completed << " times, WAIT: " << waitTime
  << " msec)" << std::endl;
  std::cout << std::endl;
  pthread_mutex_unlock(&outputlock);
  pthread_mutex_unlock(&resourceaccess); 
  
  pthread_barrier_wait(&bar5);  // Syncing thread exit with main
  pthread_exit(NULL);
}

int  main(int argc, char * argv[])  {
  // Running the main function
  int         monitorTime;
  int         taskcount;
  pthread_t   montid;
  
  cmdline_err(argc, argv);  // Error checking for cmdline input
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

  thread_main(argv[1], taskcount, &montid);  // Main thread program

  pthread_mutex_lock(&outputlock);  // Running time
  std::cout << "Running time= " << time_since_start(&start) << " msec\n"
  << std::endl;
  pthread_mutex_unlock(&outputlock);

  return 0;
}