// a4w23.h
/*
The header file for a4w23.cpp.
*/

#ifndef A4W23_H  // Guard
#define A4W23_H

// Included libraries
#include  <map>
#include  <string>
#include  <unordered_map>

// Defines

#define MAXWORD         32
#define NRES_TYPES      10
#define NTASKS          25

// Type declaration

typedef struct  {
  char                  wait[NTASKS][MAXWORD];
  char                  run[NTASKS][MAXWORD];
  char                  idle[NTASKS][MAXWORD];
  int                   waitcount;
  int                   runcount;
  int                   idlecount;
}  MONITOR_T;

typedef struct  {
  /* This is lookup for resources and counter for max amount. We use a regular
  map so that we can easily print it out for show purposes.*/
  std::map<std::string, int>                resources;
} RESOURCES_T;

typedef struct  {
  /* This struct is for monitoring the current available resources. Different
  from the above struct because the number will changed based on how many are
  available.*/

  std::unordered_map<std::string, int>      resources;
} AVAILR_T;


// Function declaration

void    cmdline_err(int argc, char * argv[]);
/* This function checks for command line errors*/
int     tokenizer(char * cmdline, std::string * tokens);
/* This function tokenizes an input string. Passes by pointer*/
void    cmdline_eater(int argc, char * argv[]);
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


#endif  /* A4W23_H */

