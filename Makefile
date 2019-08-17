CC=g++
CFLAGS=-pthread -std=c++1z

all:
	$(CC) $(CFLAGS) synchronized_2.cpp -o coursera_cpp
