# CMPUT 379 Assignment 4

name: Waridh 'Bach' Wongwandanee  
ccid: waridh  
sid:  1603722  

## Objectives

From my point of view, we learned how to use mutexes to protect critical
sections of the code (shared resources) and synchronize threads. We also needed
to prevent deadlocks using one of the methods we learnt in lecture. The method
for deadlock prevention that was used in this assignment was by only running the
program when all resources are available

## Design Overview

This program was built extremely robustly, so let me go point by point from the
beginning of the program.
1. This time, I created a header file for the program that has structure
declaration and function prototypes.
2. I use structs that contain a std c++ map/unordered_map to hold many field.
This allows our code to be very dynamic, with fast lookup time.
3. There are error handling everywhere I could have thought to place it in
4. We parse the input text file with some error handling, but also some dynamic
solutions. If the user decided to have multiple lines with the resource
allocation tag, we are able to add all of them, even if some has repeating
resource names. We also do not care about what type of whitespaces are used to
separate the tokens being used for input.
5. We read through the input file twice. The first time to get the resources,
the second the create the task threads.
6. To synchronize all the threads, we are using a combination of pthread_mutex,
pthread_barriers, and pthread_cond. Barriers were used to keep the program
moving together, mutexes to keep critical sections safe, and cond for waiting
for resources to be returned.
7. There is a global debug flag that will provide users with more information.
8. The monitor thread was created before the rest of the task threads, but it
used a barrier to halt start before all the task threads are ready as well. If
the debug flag is on, the monitor thread will also print out the resources
information, showing the max amount along with the amount that is currently
being held.
9. The task threads are created and synchronized using barriers. After all the
task threads are ready, it can access the main loop, where it will wait for all
its required resources to be ready before it changes state from wait to run.
The running task output will be printed to stdout after the program finishes its
idle stage which is placed at the end of the loop in my implementation.
10. The waiting logic used for the task thread when there isn't enough resources
uses cond. In high level, the thread will go into wait until a thread that was
running has completed running. The idea is that when a task has finished
running, some resources will be released, and all the waiting threads should
check if the resources it needs has been made available. We completed this using
pthread cond for signaling to stop waiting, and pthread mutex as our binary
semaphore.
11. The termination is synchronized again using barriers. The output that shows
the information of the threads are actually being outputted by the threads
themselves. This is not the most optimal way to print out details about things
in order, especially since you could access this information in the main thread,
however, I saw this as an opportunity to practice with more thread
synchronization. It's a little random, but we are relying on pthread mutex to
allow even progress to let the thread with the correct index access the section.
As the threads are able to create output, there will be less and less threads
trying to access the index. This is how I was able to make the threads print
their final outputs in order.

## Project Status
## Testing and Results
## Acknowledgments