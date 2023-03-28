/*
# ------------------------------
# do_work -- a program for testing user and system CPU times for CMPUT 379
#
#  Compile with:    gcc do_work.c -o do_work
#  Usage:           do_work fileName [maxIter]
#
#  Purpose:
#	This program provides a means of testing calls to the times()
#	function for printing user and system CPU times for a process,
#	and its terminated children (that the process has waited for).
#
#  Arguments:
#       Both the length of the file specified by fileName and the maxIter
#	parameter affect the consumed user and system CPU times.
#	Use a file of length >= 3 KBytes.
#	The default value of maxIter is given below.
#
#  Method:
#	The program implements a short loop that aims at consuming significant
#	user and system CPU times. The loop iterates for maxIter times.
#	Each iteration opens fileName, reads the file in small chunks and
#	evaluates an arithmetic expression, then closes the file.
#	The read system call consumes system CPU time.
#
#  Use:
#	To see the user and system CPU times consumed by the program use
#	the shell command: time do_work fileName
#
#	To see the consumed user and system CPU times of a process, copy
#	function test_times in your test program, and call the function.
#
#	To see the consumed user and system CPU times of a process' terminated
#	child, execute the program as a child process using, e.g., the system
#	call: system ("do_work fileName").
#
#  Author: Prof. Ehab Elmallah (for CMPUT 379, U. of Alberta)
# ------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define  DMAXITER  5
#define  BUFSIZE   2
// ------------------------------
void test_times(char* fileName, int maxIter)
{
    int	   i, fd;
    long   r,n;
    char   buf[BUFSIZE+1];
    for (i= 0; i < maxIter; i++) {
        if ( (fd = open (fileName, O_RDONLY)) < 0) perror ("test_times: open failed\n");
        while ((n= read(fd, buf, BUFSIZE)) > 0) { r= random() * random(); }
        close(fd);
    }	
}

// ------------------------------
int main (int argc, char *argv[] )
{
    int   maxIter;
  
    if (argc < 2 ) {
      printf ("%s: Usage: do_work fileName [maxIter]\n\n", argv[0]);
      exit(1);
    }
    maxIter= (argc == 2)?  DMAXITER : atoi(argv[2]);

    printf("%s (fileName= %s) (maxIter= %d)\n", argv[0], argv[1], maxIter);
    test_times(argv[1], maxIter);
}
