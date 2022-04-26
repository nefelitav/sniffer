#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>        
#include "../include/worker.h"
#include "../include/utilities.h"

/*
    Contains:
    - Worker funcionality
    - Urls List
*/

////////////////////////////////////////
//    Urls List methods
///////////////////////////////////////

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

////////////////////////////////////////
//    Worker functionality
///////////////////////////////////////


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

    if (close(readFile) == -1) {  
        perror("File close failed\n");
        exit(6);
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

        res = regcomp(&regex, "http://(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)", REG_EXTENDED);
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

    char occurence_str[10];
    urlNode *curr = list->head;
    while (curr != NULL) {
        sprintf(occurence_str, "%d", curr->occurences);
        write(writeFile, strcat(strcat(curr->url, " "), occurence_str), 100);
        curr = curr->next;
    }

    deleteList(list);



    
    



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
