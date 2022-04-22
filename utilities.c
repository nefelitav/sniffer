#include "utilities.h"
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

pidQueue * createPidQueue(pidQueue * head) {
    head->first = NULL;
    head->last = NULL;
    head->currSize = 0;
    return head;
}

pidNode * getFirst(pidQueue * head) {
    return head->first;
}

pidNode * getLast(pidQueue * head) {
    return head->last;
}

unsigned int getSize(pidQueue * head) {
    return head->currSize;
}

bool isEmpty(pidQueue * head) {
    return (head->currSize == 0);
}

pidQueue * push(pidQueue * head, pid_t process) {
    if (head->last == NULL) {
        // empty queue
        head->last = (pidNode*)malloc(sizeof(pidNode));
        head->last->data = process;
        head->last->availabity = false;
        head->last->next = NULL;
        head->first = head->last;
    } else {
        head->last->next = (pidNode*)malloc(sizeof(pidNode));
        head->last = head->last->next;
    }
    head->currSize++;
    return head;
}

pidNode *pop(pidQueue * head) {
    if (head->currSize == 0) {
        return NULL;
    }
    pidNode *toReturn = head->first;
    if (head->first == head->last) {
        head->last = NULL;
    }
    head->first = head->first->next;
    head->currSize--;
    return toReturn;
}

void printQueue(pidQueue * head) {
    pidNode *curr = head->first;
    printf("\n############################################\n");
    while (curr != NULL) {
        printf("%ld worker -> availability : %d\n", (long)curr->data), curr->availabity;
        curr = curr->next;
    }
    printf("############################################\n");
}

void deletePidQueue(pidQueue * head) {
    pidNode *curr = head->first;
    while (curr != NULL) {
        free(curr);
        curr = curr->next;
    }
    free(head);
}

char* getFilename(char* message) {
    char* ptr = NULL;
    char* token = strtok_r((char*)message, " ", &ptr);
    int i = 0;
    while (token != NULL && i < 2) {
        token = strtok_r(NULL, " ", &ptr);
        i++;
    }
    // printf("%s\n", token);
    return token;
}

pid_t availableWorker(pidQueue* head) {
    if (isEmpty(head)) {
        return -1;
    }
    pidNode *curr = head->first;
    while (curr != NULL) {
        if (curr->availabity == 1) {
            return curr->data;
        } 
        curr = curr->next;
    }
    return -1;

    // int status;
    // pidNode *curr = head->first;
    // while (curr != NULL) {
    //     pid_t result = waitpid(curr->data, &status, WNOHANG | WUNTRACED | WCONTINUED);
    //     if (result == 0) { // child still alive 
    //         curr = curr->next;
    //     } else if (result == -1) {
    //         perror("Child Failed\n");
    //         exit(3);
    //     } else if (result > 0) { 
    //         if (WIFSTOPPED(status)) {
    //             return curr->data; // child exited
    //         } else if (WIFCONTINUED(status)) {
    //             curr = curr->next;
    //         }
    //     }
    // }
    // return -1;
}

void pidAvailable(pid_t process, pidQueue* head) {
    pidNode *curr = head->first;
    while (curr != NULL) {
        if (curr->data == process) {
            curr->availabity = true;
            return;
        } 
        curr = curr->next;
    }
}


void worker(char* fifo) {
    int infile, outfile;
    while(1) {
        if ((infile = open(fifo, O_RDONLY)) == -1) {
            perror("File open failed\n");
            exit(4);
        }
        if ((outfile = open(strcat(fifo, ".out"), O_APPEND | O_CREAT)) == -1) {
            perror("File open failed\n");
            exit(4);
        }

        // find urls
        if((outfile = close(fd)) == -1) {  
            perror("File close failed\n");
        }  
        if((outfile = close(fd2)) == -1) {  
            perror("File close failed\n");
        }  

        kill(getpid(), SIGSTOP);
        signal(SIGSTOP, SIG_DFL);   
        // receive signal from parent to restart
        signal(SIGCONT, SIG_DFL);   
        pause();
        printf("Child process restarts\n");
        continue;
    }
}


void my_sigchld_handler(pidQueue* head)
{
    pid_t p;
    int status;

    while ((p = waitpid(-1, &status, WNOHANG)) != -1)
    {
       pidAvailable(p, head);
    }
}

