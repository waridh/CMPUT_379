CC = g++
C_FLAGS = -std=c++11 -g -Wall

p1q1: part1q1.o
	$(CC) -o $@ $<

part1q1.o: part1q1.cpp
	$(CC) $(C_FLAGS) -c $<

clean:
	rm -f p1q1 *.o
