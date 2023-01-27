C = g++
C_FLAGS = -std=c++11 -g -Wall

run: a1p2
	./a1p2

a1p2: a1p2.o
	$(CC) -o $@ $<

a1p2.o: a1p2.cpp
	$(CC) $(C_FLAGS) -c $<

clean:
	rm -f a1p2 out1 *.o
