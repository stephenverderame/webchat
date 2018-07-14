CC=g++
INC=-I../WebLib/
LIB=-L../TestServer/ -lweb
CFLAGS=-c -Wall -no-pie

all: Server

Server: Main.o
	$(CC) Main.o $(LIB) -o Server

Main.o: Main.cpp
	$(CC) $(CFLAGS) $(INC) Main.cpp

clean:
	rm -rf *o Server

