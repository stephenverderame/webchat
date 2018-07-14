CC=g++
CFLAGS=-c -Wall
INC=-I../WebLib/
LK=ar
all: lib

lib: Base.o SHA1.o Base64.o WebSocket.o Http.o Smtp.o Https.o
	$(LK) rvs libweb.a Base.o Base64.o SHA1.o WebSocket.o Http.o Smtp.o Https.o

Base.o: Base.cpp
	$(CC) $(CFLAGS) Base.cpp

SHA1.o: SHA1.cpp
	$(CC) $(CFLAGS) SHA1.cpp

Base64.o: Base64.cpp
	$(CC) $(CFLAGS) Base64.cpp

WebSocket.o: WebSocket.cpp
	$(CC) $(CFLAGS) WebSocket.cpp

Http.o: Http.cpp
	$(CC) $(CFLAGS) Http.cpp

Smtp.o: Smtp.cpp
	$(CC) $(CFLAGS) $(INC) Smtp.cpp

Https.o: Https.cpp
	$(CC) $(CFLAGS) $(INC) Https.cpp

clean:
	rm -rf *o WebLib
