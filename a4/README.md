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
solutions. If the user decided to 

## Project Status
## Testing and Results
## Acknowledgments