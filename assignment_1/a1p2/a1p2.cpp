#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <cerrno>
#include <csignal>

#include <sys/wait.h>


int main(int argc, char *argv[])  {
	// In this section, we are determining what the command line input that was
	// Chosen was.
	bool wTORsF;
	if (argc < 2)  {
		printf("Missing argument (w|s). Please run the program again.\n");
		printf("Quitting...\n");
		exit(0);
	}  else if (argv[1][0] == 'w')  {
		wTORsF = true;
	}  else if (argv[1][0] == 's')  {
		wTORsF = false;
	};

	if (wTORsF)  {
		printf("W was chosen\n");
	}  else  {
		printf("S was chosen\n");
	};

	// Spawning a child, and tasking it with a job
	//int status, pid = fork();
	//if (pid == -1)  {
//		perror("Child died, ending program.\n");
//		exit(EXIT_FAILURE);
//	}  else if (pid > 0)  {
//		printf("This is the parent process\n");
//		printf("The PID is: %i\n", getpid());
//	}  else  {
//		printf("This is a child process. Gonna run ls now.\n");
//		status = system("ls");
//	};

	// Spawning child process to run the program
	pid_t pid = fork();
	if (pid > 0)  {
		// This is the parent process
		if (wTORsF)  {
			//waitpid();
		}  else  {
			sleep(120);
			kill(pid, SIGKILL);
			printf("The program ran for two minutes\n");
			return 0;
		}
	}  else if (pid == 0)  {
		// Child process. We want to run execlp.
		execlp("./myclock", "myclock", "out1", (char *) NULL);
	};

	return 0;
}

