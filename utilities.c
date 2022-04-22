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
        printf("%ld\n", (long)curr->data);
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
        return false;
    }
    int status;
    pidNode *curr = head->first;
    while (curr != NULL) {
        pid_t result = waitpid(curr->data, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == 0) { // child still alive 
            curr = curr->next;
        } else if (result == -1) {
            perror("Child Failed\n");
            exit(3);
        } else if (result > 0) { 
            if (WIFSTOPPED(status)) {
                return curr->data; // child exited
            } else if (WIFCONTINUED(status)) {
                curr = curr->next;
            }
        }
    }
    return -1;
}

void catch(int sig) { }