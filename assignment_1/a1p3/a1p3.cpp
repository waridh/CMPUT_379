#include <iostream>
#include <sys/times.h>
#include <string>
#include <unistd.h>
#include <cstring>

int main(int argc, char *argv[])  {
	if (argc < 2)  {
		// Error handling
		std::cout << "There are not enough command-line arguments." << std::endl;
		std::cout << "Please run the program again using proper format." << std::endl;
		std::cout << "Quitting..." << std::endl;
		exit(EXIT_SUCCESS);
	};
	// Declaring for times(). Both the starting and ending times.
	long clktck=sysconf(_SC_CLK_TCK);
	clock_t time1, time2;
	struct tms cpu1; struct tms cpu2;

	// The initial time
	time1 = times(&cpu1);

	// Initializing for input
	std::string buffer;



	if (argv[1][0] == '0' && (strlen(argv[1]) == 1))  {
		std::cout << "The argument was 0" << std::endl;
		return 0;
	}  else if ((argv[1][0] == '1') && (strlen(argv[1]) == 1)  {
		std::cout << "was 1" << std::endl;
		return 0;
	}  else if ((argv[1][0] == '-') && (argv[1][1] == '1') && (strlen(argv[1]) == 2))  {
		std::cout << "neg 1" << std::endl;
		return 0;
	}  else  {
		// More error handling
		std::cout << "Command-line argument not detected" << std::endl;
	}
	return 0;
}

