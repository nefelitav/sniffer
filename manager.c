#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include "utilities.h"

#define PERMS 0666
#define BUFFER_SIZE 4096

pidQueue * queue;
pid_t pid_listener;
pid_t worker_process;
int main(int argc, char **argv) {

    char inbuf[BUFFER_SIZE];
    int p[2], nbytes = 0, code;
    pid_t pid_worker;
    char dir[100], fifo[100], folder[10];   
    char* filename = NULL;
    createPidQueue();
    memset(dir, 0, 100);
    memset(fifo, 0, 100);
    memset(inbuf, 0, BUFFER_SIZE);
    strcpy(folder, "/tmp/");

    // signal(SIGINT, sigint_handler);
    // signal(SIGCHLD, sigchld_handler);
    struct sigaction sa;
    sa.sa_handler = &sigchld_handler;
    sigaction(SIGCHLD, &sa, NULL);

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            strcpy(dir, argv[i+1]);
        }
    }
    if (pipe(p) < 0) {
        perror("Pipe Failed\n");
        exit(1);
    }
    if ((pid_listener = fork()) < 0) {
        perror("Fork Failed\n");
        exit(2);
        // worker writes
    } else if (pid_listener == 0) {
        close(p[0]);
        dup2(p[1], 1); // set pipe to stdout
        close(p[1]);
        code = execl("/usr/bin/inotifywait", "inotifywait", "-me", "create", dir, (char*)0); 
        if (code == -1) {
            perror("Execl Failed\n");
        }
       // manager reads  
    } else if (pid_listener > 0) { 
        printf("Watching directory %s\n", dir);
        close(p[1]);
        while(1) {
            // get filename
            nbytes = read(p[0], inbuf, BUFFER_SIZE);
            printf("%.*s", nbytes, inbuf);
            getFilename((char*)&inbuf, &filename);
            printf("%s\n", filename);
            // strcpy(fifo, strcat(folder, filename));
            // int res = mkfifo(fifo, PERMS);
            // if (res < 0) {
            //     perror("can't create fifo");
            // } 
            if (availableWorker() != -1) {
                printf("available\n");
                // send signal to child to wake up
                kill(availableWorker(), SIGCONT);
            } else { 
                worker_process = getpid();
                // no available workers -> create one
                if ((pid_worker = fork()) < 0) {
                    perror("Fork Failed\n");
                    exit(2);
                } 
                else if (pid_worker == 0) {
                printf("worker\n");
                    worker(fifo);
                    exit(0); // child exits
                } 
                else if (pid_worker > 0) {
                printf("parent\n");
                    push(pid_worker);

                }
            }
        }
    }
    deletePidQueue();
    exit(0);
}

