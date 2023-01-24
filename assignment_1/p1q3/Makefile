CC = g++
C_FLAGS = -std=c++11 -g -Wall

p1q3r: p1q3.o
	$(CC) -o $@ $<

p1q3.o: p1q3.cpp
	$(CC) $(C_FLAGS) -c $<

clean:
	rm -f p1q3r *.o
  
