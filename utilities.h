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

pidQueue * createPidQueue(pidQueue * head);
pidNode * getFirst(pidQueue * head);
pidNode * getLast(pidQueue * head);
unsigned int getSize(pidQueue * head);
bool isEmpty(pidQueue * head);
pidQueue * push(pidQueue * head, pid_t process);
pidNode *pop(pidQueue * head);
void printQueue(pidQueue * head);

char* getFilename(char* message);
pid_t availableWorker(pidQueue* head);