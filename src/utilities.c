#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>       
#include "../include/utilities.h"
#include "../include/worker.h" 

/*
    Contains:
    - Queue with workers's pids - functions
    - Signal handlers
    - Useful functions
*/

////////////////////////////////////////
//    Workers queue
///////////////////////////////////////

void createPidQueue() {
    queue = malloc(sizeof(pidQueue));
    queue->first = NULL;
    queue->last = NULL;
    queue->currSize = 0;
}

bool isEmpty() {
    return (queue->currSize == 0);
}

void push(pid_t process) {
    if (queue->last == NULL) {
        // empty queue
        queue->last = (pidNode*)malloc(sizeof(pidNode));
        queue->last->data = process;
        queue->last->availabity = false;
        queue->last->next = NULL;
        queue->first = queue->last;
    } else {
        queue->last->next = (pidNode*)malloc(sizeof(pidNode));
        queue->last = queue->last->next;
        queue->last->data = process;
        queue->last->availabity = false;
        queue->last->next = NULL;
    }
    queue->currSize++;
}

pidNode *pop() {
    if (queue->currSize == 0) {
        return NULL;
    }
    pidNode *toReturn = queue->first;
    if (queue->first == queue->last) {
        queue->last = NULL;
    }
    queue->first = queue->first->next;
    queue->currSize--;
    return toReturn;
}

void printQueue() {
    pidNode *curr = queue->first;
    printf("\n############################################\n");
    while (curr != NULL) {
        printf("pid = %ld, availability = %d\n", (long)curr->data, curr->availabity);
        curr = curr->next;
    }
    printf("############################################\n");
}

void deletePidQueue() {
    pidNode *curr = queue->first;
    pidNode *next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(queue);
}

bool isAvailable(pid_t process) {
    if (isEmpty(queue)) {
        return false;
    }
    pidNode *curr = queue->first;
    while (curr != NULL) {
        if (curr->data == process) {
            return curr->availabity;
        } 
        curr = curr->next;
    }
    return false;
}

pid_t availableWorker() {
    if (isEmpty(queue)) {
        return -1;
    }
    pidNode *curr = queue->first;
    while (curr != NULL) {
        if (curr->availabity == 1) {
            return curr->data;
        } 
        curr = curr->next;
    }
    return -1;
}

void pidAvailable(pid_t process) {
    pidNode *curr = queue->first;
    while (curr != NULL) {
        if (curr->data == process) {
            curr->availabity = true;
            return;
        } 
        curr = curr->next;
    }
}

void pidUnavailable(pid_t process) {
    pidNode *curr = queue->first;
    while (curr != NULL) {
        if (curr->data == process) {
            curr->availabity = false;
            return;
        } 
        curr = curr->next;
    }
}

////////////////////////////////////////
//    Signal handlers
///////////////////////////////////////

void sigchld_handler(int signum) {
    pid_t p;
    if ((p = waitpid(-1, 0, WUNTRACED | WCONTINUED)) != -1) {
        if (p == listener_process) {
            signal(SIGCHLD, SIG_IGN);
            return;
        }
        printf("child id = %d\n", p);
    } 
    if (isAvailable(p)) {
        printf("catch start\n");
        pidUnavailable(p);
    } else {
        printf("catch stop\n");
        pidAvailable(p);
    }
    printQueue();
}


void sigint_handler(int signum) {
    printf("KILL\n");
    kill(pid_listener, signum);
    pidNode *curr = queue->first;
    while (curr != NULL) {
        kill(curr->data, SIGCONT);
        kill(curr->data, signum);
        curr = curr->next;
    }

    deletePidQueue();
    exit(0);
}

////////////////////////////////////////
//    Useful functions
///////////////////////////////////////


void getFilename(char* message, char** output) {
    char* ptr = NULL;
    *output = strtok_r((char*)message, " ", &ptr);
    int i = 0;
    while (*output != NULL && i < 2) {
        *output = strtok_r(NULL, " ", &ptr);
        i++;
    }
    return;
}

void pathWithSlash(char *path) {
    if (strcmp(&path[strlen(path) - 1], "/") != 0) {
        path[strlen(path)] = '/';
    }
}
