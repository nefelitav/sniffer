#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

typedef struct pidNode pidNode;

struct pidNode {
    pid_t data;
    bool availabity;
    pidNode *next;
};

typedef struct pidQueue pidQueue;

struct pidQueue {
    pidNode *first;
    pidNode *last;
    unsigned int currSize;
};

void createPidQueue();
void deletePidQueue();
pidNode * getFirst();
pidNode * getLast();
unsigned int getSize();
bool isEmpty();
void push(pid_t process);
pidNode *pop();
void printQueue();

typedef struct urlNode urlNode;

struct urlNode {
    char* url;
    int occurences;
    urlNode *next;
};

typedef struct urlList urlList;

struct urlList {
    urlNode *head;
};

void addUrl(urlList* list, char* newUrl);
bool alreadyIn(urlList* list, char* newUrl);
void deleteList(urlList* list);
void printList(urlList* list);

void getFilename(char* message, char** output);
pid_t availableWorker();
void sigchld_handler(int signum);
void sigint_handler(int signum);
void findUrls();
void worker();
void pidAvailable(pid_t process);
void pidUnavailable(pid_t process);
bool isAvailable(pid_t process);
void pathWithSlash(char *givenPath);


extern pidQueue * queue;
extern pid_t pid_listener;
extern pid_t worker_process;
extern pid_t listener_process;
extern char dir[100];