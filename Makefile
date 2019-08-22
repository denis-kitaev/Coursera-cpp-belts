CC=g++
CFLAGS=-pthread -std=c++1z -Wall

all:
	$(CC) $(CFLAGS) zoo.cpp -o coursera_cpp
