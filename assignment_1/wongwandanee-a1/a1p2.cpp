#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <csignal>

#include <sys/wait.h>


int main(int argc, char *argv[])  {
	// In this section, we are determining what the command line input that was
	// Chosen was.
	bool wTORsF;  // This boolean is the w|s argument switch
	if (argc < 2)  {
		// Error handling for not having enough arguments
		printf("Missing argument (w|s). Please run the program again.\n");
		printf("Quitting...\n");
		exit(0);
	}  else if (argv[1][0] == 'w')  {
		wTORsF = true;
	}  else if (argv[1][0] == 's')  {
		wTORsF = false;
	};

	// Spawning child process to run the program
	pid_t pid = fork();
	if (pid > 0)  {
		// This is the parent process
		if (wTORsF)  {
			waitpid(pid, NULL, 0);
			// Debug prints
			// printf("The child process has killed itself\n");
			return 0;
		}  else  {
			// Letting the child program run for two minutes
			sleep(120);
			kill(pid, SIGKILL);  // Kill the child process

			// Debug print
			// printf("The program ran for two minutes\n");
			return 0;
		}
	}  else if (pid == 0)  {
		// Child process. We want to run execlp.
		execlp("./myclock", "myclock", "out1", (char *) NULL);
		exit(EXIT_SUCCESS);
	};

	return 0;
}

