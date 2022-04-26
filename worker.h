#ifndef WORKER
#define WORKER

#include <stdio.h> 
#include <unistd.h>
#include <sys/types.h>
#include <stdbool.h>


////////////////////////////////////////
//   Url List - structs & methods
///////////////////////////////////////

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

////////////////////////////////////////
//    Worker functionality
///////////////////////////////////////

void findUrls();
void worker();

#endif