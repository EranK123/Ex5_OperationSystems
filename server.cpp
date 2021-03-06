/*
** server.c -- a stream socket server demo
*/
#include <fcntl.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
using namespace std;

#define PORT "3490"  // the port users will be connecting to

#define BACKLOG 10   // how many pending connections queue will hold



// -------------- Stack Imp ----------- //

struct Node {
   char* data;
   struct Node *next;
};

struct Stack
{
    Node *n;
    Stack *loc;
};

Stack *stac;

// struct Node* top = NULL;
void push(string val) {
    Node *t = (Node*)stac->loc;
    t->next = nullptr;
    t->data = (char*)stac->loc + sizeof(Node);
    int idx = 0;
    for(int i = 0; i < val.length(); i++){
        t->data[idx] = val[i];
        idx++;
    }
    t->data[idx] = '\0';
    
    if(stac->n == NULL){
        stac->n = t;
    }else{
        t->next = stac->n;
        stac->n = t;
    }
    stac->loc = stac->loc + val.length() + 1;

}
void pop() {
    if(stac->n == nullptr){
   cout<<"Stack Underflow"<<endl;
   return;
   } else {
    stac->n = stac->n->next;
    }
}

char* peek(){
    cout << "Peek entered" << endl;
    if(stac->n != nullptr){
    return stac->n->data;
    }
    return (char*)"Stack is empty";
}

// string display() {
//    string s; 
//    struct Node* ptr;
//    if(stac->n==NULL)
//    return "\n";
//    else {
//       ptr = stac->n;
//       while (ptr != NULL) {
//          s += ptr->data + " ";
//          ptr = ptr->next;
//       }
//    }
//    s += '\n';
//    return s;
// }



// -------------- Server Imp ----------- //
struct flock lock1;
char client_message[1024];
void stackProcess(int arg)
{
   
    memset (&lock1, 0, sizeof(lock1));
    lock1.l_type = F_WRLCK;
    fcntl (arg, F_SETLKW, &lock1);
    recv(arg , client_message , 1024 , 0);
   string commnad = client_message;
   if(commnad.substr(0, 4).compare("PUSH") == 0){
       push(commnad.substr(5, commnad.length()));
   }else if(commnad.substr(0, 3).compare("POP") == 0){
       pop();
   }else if(commnad.substr(0, 3).compare("TOP") == 0){
       string data = "OUTPUT: " + string(peek()) + '\n';
       cout << "data:" << data << endl;
       send(arg, data.c_str(), strlen(data.c_str()), 0);
   }
//    else if(commnad.substr(0, 4).compare("DISP") == 0){
//         //  string out = "Stack elements are: " + display() + '\n';
//         // string out = "Stack elements are: " + display();
//          send(newSocket, out.c_str(), strlen(out.c_str()), 0);
//    }  

sleep(1);
lock1.l_type = F_UNLCK;
fcntl (arg, F_SETLKW, &lock1);
close(arg);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");
    stac = (Stack*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    stac->n = nullptr;
    stac->loc = stac + sizeof(struct Stack);
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            stackProcess(new_fd);
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
    }

    return 0;

}