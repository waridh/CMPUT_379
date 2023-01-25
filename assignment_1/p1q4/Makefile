CC = g++
C_FLAGS = -std=c++11 -g -Wall

p1q4: p1q4.o
	$(CC) -o $@ $<

p1q4.o: p1q4.cpp
	$(CC) $(C_FLAGS) -c $<

clean:
	rm -f p1q4 *.o
