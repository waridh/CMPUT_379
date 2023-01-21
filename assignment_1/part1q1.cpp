// Assignment 1, part 1, question 1

#include <cstring>
#include <iostream>

using namespace std;

// Driver code



int main()  {
  struct PCB_t  {
    int integerboi;
    int intb;
    char str[];
  };
  PCB_t pcb;
  memset(&pcb, 0, sizeof(pcb));
  cout << pcb.integerboi << endl;
  cout << pcb.str << endl;
  cout << pcb.intb << endl;
  return 0;
}
