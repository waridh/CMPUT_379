#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

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
  size_t len = 0;
  ssize_t read;
  read = getline(fp, line, '\n');
  sprintf(indicator, "%04d", i); 
  std::cout << "[" << indicator << "]: '" << line << "'" << std::endl;
  
};

void lineout_controller(char * inFile, int * nLine)  {
  /* This function attempts to control the line output*/
  std::ifstream  fp;
  int           i, count = 1;
  fp.open(inFile);
  if (fp.is_open())  {
    for (i = 0; i < *nLine; i++)  {
    readLine(fp, count);
    count++;
    }
  };
};

int main(int argc, char * argv[])  {
  int nLine, delay;

  input_check(&argc);  // Running a check for correct argument amount
  disp_arg(argv);  // Disp the initial string.
  
  char * inFile = argv[2];
  nLine = atoi(argv[1]); delay = atoi(argv[3]);  // Turning cmd line arg to var

  //===========================================================================
  // Getting the file line
  lineout_controller(inFile, &nLine);


  return 0;
};