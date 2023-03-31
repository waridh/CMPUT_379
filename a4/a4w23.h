// a4w23.h
/*
The header file for a4w23.cpp.
*/

#ifndef A4W23_H  // Guard
#define A4W23_H

// Included libraries
#include  <map>
#include  <semaphore.h>
#include  <set>
#include  <string>
#include  <unordered_map>

// Defines

#define   MAXWORD         32
#define   NRES_TYPES      10
#define   NTASKS          25

// Type declaration

typedef struct  {
  /* Structure for holding the monitor task manager*/
  std::set<std::string>   wait;
  std::set<std::string>   run;
  std::set<std::string>   idle;
  uint                    waitcount;
  uint                    runcount;
  uint                    idlecount;
}  MONITOR_T;

typedef struct  {
  /* This is lookup for resources and counter for max amount. We use a regular
  map so that we can easily print it out for show purposes. It will become
  readonly after it's initialization*/
  std::map<std::string, int>                resources;
} RESOURCES_T;

typedef struct  {
  /* This struct is for monitoring the current available resources. Different
  from the above struct because the number will changed based on how many are
  available.*/

  std::unordered_map<std::string, int>                resources;
} AVAILR_T;

typedef struct  {
  /* This structure is created to send the required structs into the thread
  creation process, so each task knows how much resources it needs. We are
  going to use an array. I want to index the resources and call those here.

  Looking at this structure, we have everything information we could possibly
  need for the threads
  
  Note:
    The time stuff is specified as millisecs*/
  uint                                      busyTime; // Time spent by task
  uint                                      idleTime; // Break time
  uint                                      rtypes;  // The amount of types
  int                                       idx;
  std::string                               name;
  std::map<std::string, int>                requiredr;  // Using map cause order
} THREADREQUIREMENTS;

typedef struct  {
  /*Making a megastruct that will hold both the pthread_t and the information */
  THREADREQUIREMENTS                        info;
  pthread_t                                 tid;
}  THREADESSENCE;

typedef struct  {
  /* The big map using name as a key and the value being all information
  relating to a thread. This should make our program very dynamic*/
  std::map<std::string, THREADESSENCE>      thread;
}  THREADMAP;


// Function declaration

// Errors

void    cmdline_err(int argc, char * argv[]);
/* This function checks for command line errors*/

void    barrier_err();
/* For when we cannot initialize the barrier obj*/

void    thread_init_err(THREADREQUIREMENTS * info, pthread_t * tid);
/* Error handling at the initialization of each thread. When there are too
many resource requirements and when the resource does not exist*/

// Utilities

void    timeprint(
  clock_t &time1,
  clock_t &time2,
  struct tms &cpu1,
  struct tms &cpu2
  );
/* This function creates output on how much cpu time was used*/

double  time_since_start(clock_t * startTime);
/* This function is created to return the time in milliseconds since the start*/

int     tokenizer(char * cmdline, std::string * tokens);
/* This function tokenizes an input string. Passes by pointer*/

int     cmdline_eater(int argc, char * argv[]);
/* This function actually parses the input given in the command line arg*/

void    resource_gatherer(std::string * resource_line, int tokenscount);
/* This function reads the input file and allocates the resources into the
program*/

void *  monitor_thread(void * arg);
/* This function is the monitor thread*/

void *  task_thread(void * arg);
/* This function is the task thread*/

int     colon_tokenize(std::string * pair, std::string * name);
/* This function takes in a colon separated pair in the form of a string
pointer, and then separate it into the name and number values. The string name
is passed back via pointer, and the value is returned*/

void    thread_creation(
  char *                filename,
  THREADMAP *           thread_map
  );
/* Second readthrough of the file so that we can create the thread separately
from the resource allocation. Allows for easier synchronization too?*/

void    thread_main(
  char *        filename,
  int           tasksamount,
  pthread_t *   monitorthread
  );
/* The main function for threading. Creates threads and also waits for them to
exit to retrieve resources and keep the program synchronized.

  We want to store the information that is eventually passed into the thread
  in this function.
  
  We also want to store the tids here. I am planning to use dynamic allocation
  for this purpose.*/

void    thread_creator(
  std::string *           task_list,
  int                     idx,
  int                     tokenscount,
  THREADMAP *             threadm
  );
/* This function will create the task threads from the line present in the
input. The goal here is to launch the thread using this function*/

void monitor_signal(int signum);
/* Sending a signal that will just terminal the monitor thread. Good for sync*/

// Clean up

void barrier_ender();
/* Clears the barriers*/

#endif  /* A4W23_H */

