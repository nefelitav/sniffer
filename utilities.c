#include "utilities.h"
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>        


void createPidQueue() {
    queue = malloc(sizeof(pidQueue));
    queue->first = NULL;
    queue->last = NULL;
    queue->currSize = 0;
}

pidNode * getFirst() {
    return queue->first;
}

pidNode * getLast() {
    return queue->last;
}

unsigned int getSize() {
    return queue->currSize;
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

void findUrls(int infile, int outfile) {
    // regex_t regex;
    // char msgbuf[100];
    // int infile, outfile, res = 0;
    // find urls
    // if ((infile = open(fifo, O_RDONLY)) == -1) {
    //     perror("File open failed\n");
    //     exit(4);
    // }
    // if ((outfile = open(strcat(fifo, ".out"), O_APPEND | O_CREAT)) == -1) {
    //     perror("File open failed\n");
    //     exit(5);
    // }
    // res = regcomp(&regex, "(http://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)", 0);
    // if (res) {
    //     fprintf(stderr, "Could not compile regex\n");
    //     exit(1);
    // }
    // res = regexec(&regex, "-------------input--------------", 0, NULL, 0);
    // if (!res) {
    //     // match
    //     //write(outfile, (char *)str_write, 10); 
    // } else if (res == REG_NOMATCH) {
    //     // no match
    // } else {
    //     regerror(res, &regex, msgbuf, sizeof(msgbuf));
    //     perror("Regex failed\n");
    //     exit(1);
    // }
    // regfree(&regex);
    // if((outfile = close(fd)) == -1) {  
    //     perror("File close failed\n");
    //     exit(6);
    // }  
    // if((outfile = close(fd2)) == -1) {  
    //     perror("File close failed\n");
    //     exit(7);
    // }  
}

void worker(char* fifo) {

    while(1) {
        // findUrls(infile, outfile);
        printf("stopped\n");
        // send signal to myself to stop
        if (raise(SIGSTOP) == -1) {
            perror("Raise failed\n");
            exit(8);
        }
        printf("restarted\n");
    }
}

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
        kill(curr->data, signum);
        curr = curr->next;
    }

    exit(0);
}

