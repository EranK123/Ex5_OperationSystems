CFLAGS=-Wall
COMP = clang++

all: Client Server 

Client: Client.o
	$(COMP) $(CFLAGS) Client.o -o Client 

Server: Server.o
	$(COMP) $(CFLAGS) Server.o -o Server 

test: test.o
	$(COMP) $(CFLAGS) test.o -o test

Server.o: server.cpp
	$(COMP) -c $(CFLAGS) server.cpp 

Client.o: client.cpp
	$(COMP) -c $(CFLAGS) client.cpp 

.PHONY: clean all

clean:
	rm *.o Client Server
			