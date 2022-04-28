#ifndef UTILITIES
#define UTILITIES

#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>

////////////////////////////////////////
//   Workers queue - structs & methods
///////////////////////////////////////

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
bool isEmpty();
void push(pid_t process);
pidNode *pop();
void printQueue();
pid_t availableWorker();
void pidAvailable(pid_t process);
void pidUnavailable(pid_t process);
bool isAvailable(pid_t process);
bool isWorker(pid_t process);


////////////////////////////////////////
//    Signal handlers
///////////////////////////////////////

void sigchld_handler(int signum);
void sigint_handler(int signum);

////////////////////////////////////////
//    Useful functions
///////////////////////////////////////

void getFilename(char* message, char** output);
void pathWithSlash(char *givenPath);

////////////////////////////////////////
//    Global variables
///////////////////////////////////////

extern pidQueue * queue;
extern pid_t pid_listener;
extern pid_t pid_manager;
extern pid_t worker_process;
extern char* dir;

#endif
