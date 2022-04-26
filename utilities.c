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


void addUrl(urlList* list, char* newUrl) {
    urlNode *curr = NULL;
    if (alreadyIn(list, newUrl) == true) {
        return;
    } else {
        curr = malloc(sizeof(urlNode));
        curr->url = malloc(sizeof(char)*(strlen(newUrl) + 1));
        strcpy(curr->url, newUrl);
        curr->occurences = 1;
        curr->next = list->head;
        list->head = curr;
    }

}
bool alreadyIn(urlList* list, char* newUrl) {
    urlNode *curr = list->head;
    urlNode *next;
    while (curr != NULL) {
        next = curr->next;
        if (strcmp(curr->url, newUrl) == 0) {
            curr->occurences++;
            return true;
        }
        curr = next;
    }
    return false;
}

void printList(urlList* list) {
    printf("\n############################################\n");
    urlNode *curr = list->head;
    while (curr != NULL) {
        printf("%s : %d occurences\n", curr->url, curr->occurences);
        curr = curr->next;
    }
    printf("\n############################################\n");
}

void deleteList(urlList* list) {
    urlNode *curr = list->head;
    urlNode *next;
    while (curr != NULL) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    free(list);
}

void findUrls() {
    int readPipe, readFile, writeFile, res = 0;
    char filename[100], fifo[100], folder[100];   
    memset(fifo, 0, 100);
    strcpy(folder, "/tmp/");
    char mypid[100];
    sprintf(mypid, "%d", getpid());
    strcpy(fifo, strcat(folder, mypid));


    // read filename from pipe
    if ((readPipe = open(fifo, O_RDONLY)) == -1) {
        perror("File open failed\n");
        exit(4);
    }
    read(readPipe, filename, sizeof(filename));
    if (close(readPipe) == -1) {  
        perror("File close failed\n");
        exit(6);
    } 
    filename[strcspn(filename, "\n")] = 0;
    printf("Filename = -%s-\n", filename); // /mnt/c/Users/ntavoula/Desktop/tmp/x

    // find urls
    if ((readFile = open(filename, O_RDONLY)) == -1) {
        perror("File open failed\n");
        exit(4);
    }


    off_t currentPos = lseek(readFile, (size_t)0, SEEK_CUR);
    off_t size = lseek(readFile, (size_t)0, SEEK_END);
    lseek(readFile, currentPos, SEEK_SET); 
    char buffer[size], text[size];
    int len;
    memset(text, 0, size);

    while((len = read(readFile, buffer, size)) != 0) {     
        strcpy(text, buffer);
        break;           
    }
    printf("the string is = %s.\n", text); 
    char * urlStart = text;
    char * url;
    regex_t regex;
    char msgbuf[100];
    urlList* list = malloc(sizeof(urlList));

    while ((urlStart = strstr(urlStart, "http")) != NULL) {
        url = strtok_r(urlStart, " ", &urlStart);
        printf("the url is = %s.\n", url); 

        res = regcomp(&regex, "http://(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)", REG_EXTENDED|REG_NOSUB);
        if (res) {
            fprintf(stderr, "Could not compile regex\n");
            exit(1);
        }
        res = regexec(&regex, url, 0, NULL, 0);
        printf("res %d\n", res);
        if (!res) {
            // match
            addUrl(list, url);
        } else if (res == REG_NOMATCH) {
            // no match
        } else {
            regerror(res, &regex, msgbuf, sizeof(msgbuf));
            perror("Regex failed\n");
            exit(1);
        }
        regfree(&regex);
    } 

    printList(list);
    // urlNode *curr = list->head;
    // while (curr != NULL) {

    //     curr->url, curr->occurences
    //     curr = curr->next;
    // }




    deleteList(list);

    if (close(readFile) == -1) {  
        perror("File close failed\n");
        exit(6);
    } 

    char* ptr = filename;
    char * output;
    char * temp;
    while ((temp = strtok_r(ptr, "/", &ptr))) {
        if (temp == NULL) {
            break;
        }
        output = temp;
    }
    char * newFilename = strcat(output, ".out");
    printf("-%s-\n", newFilename);
    memset(folder, 0, 100);
    strcpy(folder, "/tmp/");
    char * newFile = strcat(folder, newFilename);
    printf("-%s-\n", newFile);

    if ((writeFile = open(newFile, O_CREAT | O_RDWR | O_APPEND)) == -1) {
        perror("File open failed\n");
        exit(5);
    }

    



    if (close(writeFile) == -1) {  
        perror("File close failed\n");
        exit(7);
    }  
}

void worker() {
    while(1) {
        findUrls();
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
        kill(curr->data, SIGCONT);
        kill(curr->data, signum);
        curr = curr->next;
    }

    deletePidQueue();
    exit(0);
}


void pathWithSlash(char *path) {
    if (strcmp(&path[strlen(path) - 1], "/") != 0) {
        path[strlen(path)] = '/';
    }
}
