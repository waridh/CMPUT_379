#include <fstream>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//=============================================================================
// Structures
typedef struct  {
  char*   inFile;
  int     nLine;
  int     delay;
} super_struct;

//=============================================================================
// Error outputs

void input_check(int * argc)  {
  /* This is the comand line argument check. Makes sure that the user is using
  it correctly. If I have time in the future, we will also check for type*/
  if (* argc != 4)  {
    std::cout << "Not enough inputs. Please follow the usage" << std::endl <<
    "a2p1 nLine inputFile delay" << std::endl << "Where nline is the number of"
    " lines that will be displayed on the screen, inputFile is the input file,"
    " and delay is the time in milliseconds that the program will stop for." <<
    std::endl;
    exit(EXIT_FAILURE);
  }
  
  return;
}

void print_file_error()  {
  /* Mostly for when we cannot open a file*/
  std::cout << "Failed to open the file, please make sure that the file exists"
  << std::endl;
  exit(EXIT_FAILURE);
}

void disp_arg(char * argv[])  {
  /* Following the output given in the example output, I am mimicing that by
  giving the user what the output from */
  std::cout << "a2p1 starts: (nLine=" << argv[1] << ", inFile='" 
  << argv[2] << "', delay= " << argv[3] << ")" << std::endl;
};

void readLine(std::ifstream &fp, int i)  {
  /* This function just outputs a single line read from the file*/
  std::string line;
  char indicator[] = "0000";
  std::getline(fp, line);
  sprintf(indicator, "%04d", i); 
  std::cout << "[" << indicator << "]: '" << line << "'" << std::endl;
  
};

void * lineout_controller(void * input_args)  {
  /* This function attempts to control the line output*/
  /* TODO:
        Need to figure out how we want to close the file*/

  super_struct * arguments = (super_struct *) input_args;
  std::ifstream  fp(arguments -> inFile);
  int           i, count = 1;
  std::cout << "User command:" << std::endl;
  sleep(5);

  return NULL;
};

void thread_controller(super_struct arguments)  {
  /* This function creates and controls thread that will run the line out and
  the command line input*/
  pthread_t       tid1;
  pthread_attr_t  attr1;
  int             err;
  int             i, count = 1;
  std::ifstream   fp(arguments.inFile);
  void*           tret;
  // Setting the default attribute of the thread
  pthread_attr_init(&attr1);
  // Running open check
  if (!fp.is_open())  {
    print_file_error();
  };
  // Running the main loop
  while (!fp.eof())  {
    for (i = 0; i < arguments.nLine; i++)  {
      if (fp.eof())  {
        break;
      }
    readLine(fp, count);
    count++;

    }

    std::cout << std::endl << "*** Entering a delay period of "
    << arguments.delay << "msec" << std::endl << std::endl;

    // Creating the thread
    err = pthread_create(
      &tid1,
      &attr1,
      lineout_controller,
      (void *) &arguments);
    if (err != 0)  {
      std::cout << "Unable to create thread" << std::endl;
      exit(EXIT_FAILURE);
    }
    err = pthread_join(tid1, &tret);
  };

  
  
  return;
};

int main(int argc, char * argv[])  {
  super_struct arguments;

  input_check(&argc);  // Running a check for correct argument amount
  disp_arg(argv);  // Disp the initial string.
  
  (arguments).inFile = argv[2];
  (arguments).nLine = atoi(argv[1]); (arguments).delay = atoi(argv[3]);
  // Turning cmd line arg to var

  //===========================================================================
  // Running the thread controller to control thread actions
  
  thread_controller(arguments);


  return 0;
};