CFLAGS=-Wall
COMP = clang++

all: client server 

client: client.o
	$(COMP) $(CFLAGS) client.o -o client 

server: server.o
	$(COMP) $(CFLAGS) server.o -o server 

server.o: server.cpp
	$(COMP) -c $(CFLAGS) server.cpp 

client.o: client.cpp
	$(COMP) -c $(CFLAGS) client.cpp 

.PHONY: clean all

clean:
	rm *.o client server
			