CC=g++
CFLAGS=-pthread -std=c++1z -Wall

all:
	$(CC) $(CFLAGS) print_stats_bugs/good.cpp -o coursera_cpp
