CC = g++
C_FLAGS = -std=c++11 -g -Wall

we7_solution: climbing.o
	$(CC) -o $@ $<

climbing.o: climbing.cpp
	$(CC) $(C_FLAGS) -c $<

clean:
	rm -f we7_solution *.o
