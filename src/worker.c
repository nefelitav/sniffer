#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>        
#include "../include/worker.h"
#include "../include/utilities.h"

#define PERMS 0666

/*
    Contains:
    - Worker funcionality
    - Urls List
*/

////////////////////////////////////////
//    Urls List methods
///////////////////////////////////////

// add new url to list
void addUrl(urlList* list, char* newUrl) {
    urlNode *curr = NULL;
    if (alreadyIn(list, newUrl) == true) {
        return;
    } else {
        curr = malloc(sizeof(urlNode));
        curr->url = malloc(sizeof(char)*(strlen(newUrl) + 1 + 10));
        strcpy(curr->url, newUrl);
        curr->occurences = 1;
        curr->next = list->head;
        list->head = curr;
    }

}

// url already in list -> increment occurences
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
    urlNode *curr;
    curr = list->head;
    while (curr != NULL) {
        printf("%s : %d occurences\n", curr->url, curr->occurences);
        curr = curr->next;
    }
    printf("\n############################################\n");
}

void deleteList(urlList* list) {
    urlNode *curr = NULL;
    curr = list->head;
    urlNode *next = NULL;
    while (curr != NULL) {
        next = curr->next;
        free(curr->url);
        free(curr);
        curr = next;
    }
    free(list);
}

////////////////////////////////////////
//    Worker functionality
///////////////////////////////////////


void findUrls() {
    int readPipe, readFile, writeFile, res = 0;
    char filename[100], *fifo, folder[10], mypid[100];   
    fifo = malloc(sizeof(char) * (110 + 1));
    memset(fifo, 0, 110 + 1);
    strcpy(folder, "/tmp/");
    sprintf(mypid, "%d", getpid());
    strcpy(fifo, strcat(folder, mypid));
    sleep(0.5);
    // read filename from pipe
    if ((readPipe = open(fifo, O_RDONLY)) == -1) {
        perror("Failed to open named pipe\n");
        exit(1);
    }
    if (read(readPipe, filename, sizeof(filename)) == -1) {
        perror("Failed to read from named pipe\n");
        exit(1);
    }
    if (close(readPipe) == -1) {  
        perror("Failed to close named pipe\n");
        exit(1);
    } 
    free(fifo);
    
    filename[strcspn(filename, "\n")] = 0;
    printf("Filename = %s\n", filename); 

    // find urls in that file
    if ((readFile = open(filename, O_RDONLY)) == -1) {
        perror("File open failed\n");
        exit(1);
    }

    // get file size
    off_t currentPos = lseek(readFile, (size_t)0, SEEK_CUR);
    off_t size = lseek(readFile, (size_t)0, SEEK_END);
    lseek(readFile, currentPos, SEEK_SET); 
    char buffer[size], text[size*2];
    int len;
    memset(text, 0, 2*size);
    memset(buffer, 0, size + 1);


    // get whole file in a string
    while((len = read(readFile, buffer, size)) != 0) {   
        if (len == -1) {
            perror("Failed to read from file\n");
            exit(1);
        }  
        strcpy(text, buffer);
        break;           
    }

    if (close(readFile) == -1) {  
        perror("File close failed\n");
        exit(1);
    } 

    char * urlStart = text;
    char * url;
    regex_t regex;
    char msgbuf[100];
    urlList* list = malloc(sizeof(urlList));
    memset(list, 0, sizeof(urlList));

    // if there is an http in file
    while ((urlStart = strstr(urlStart, "http")) != NULL) {
        url = strtok_r(urlStart, " ", &urlStart);

        // check if it is a valid url
        if (regcomp(&regex, "http://(www.)?[a-zA-Z0-9@:%._\\+~#?&//=-]{1,256}\\.[a-z]{1,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)", REG_EXTENDED)) {
            perror("Could not compile regex\n");
            exit(1);
        }
        // execute regex
        res = regexec(&regex, url, 0, NULL, 0);
        if (!res) {
            // match -> add url to list
            char* wwwPtr = strstr(url, "www.");
            char* httpPtr = strstr(url, "http://");
            if (wwwPtr != NULL) {
                url = (char*)(wwwPtr + 4);
            } else {
                url = (char*)(httpPtr + 7);
            }
            url = strtok_r(url, ":", &url);
            url = strtok_r(url, "#", &url);
            url = strtok_r(url, "?", &url);
            url = strtok_r(url, "/", &url);
            
            printf("the url is = %s.\n", url); 
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


    memset(folder, 0, 10);
    strcpy(folder, "/tmp/");
    char * newFile = strcat(folder, newFilename);
    printf("Writing in %s\n", newFile);


    // create .out file
    if ((writeFile = open(newFile, O_CREAT | O_RDWR, PERMS)) == -1) {
        perror("Failed to open .out file\n");
        exit(1);
    }

    char occurence_str[10];
    urlNode *curr = list->head;
    // for every url -> write to .out file
    while (curr != NULL) {
        sprintf(occurence_str, "%d", curr->occurences);
        strcat(occurence_str, "\n");
        strcat(curr->url, " ");
        strcat(curr->url, occurence_str);
        if (write(writeFile, curr->url, strlen(curr->url)) == -1) {
            perror("Failed to write to .out file\n");
            exit(1);
        }
        curr = curr->next;
    }

    deleteList(list);


    if (close(writeFile) == -1) {  
        perror("Failed to close .out file\n");
        exit(1);
    }  
}

void worker() {
    while(1) {
        findUrls();
        printf("Worker %d stopped\n", getpid());
        // send signal to myself to stop
        if (raise(SIGSTOP) == -1) {
            perror("Raise failed\n");
            exit(1);
        }
        printf("Worker %d restarted\n", getpid());
    }
}
