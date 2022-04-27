#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include "../include/utilities.h"
#include "../include/worker.h"

#define PERMS 0666
#define BUFFER_SIZE 1000

pidQueue * queue;
pid_t pid_listener;
pid_t worker_process;
pid_t listener_process;
char dir[100];

int main(int argc, char **argv) {

    int p[2], nbytes = 0, infile, arg = 0;
    pid_t pid_worker;
    char inbuf[BUFFER_SIZE], fifo[100], folder[100], path[100], mypid[100];   
    char* filename = NULL;

    // create workers queue
    createPidQueue(); 

    memset(path, 0, 100);
    memset(dir, 0, 100);
    memset(inbuf, 0, BUFFER_SIZE);

    // stop when i receive ctrl+c
    signal(SIGINT, sigint_handler);

    // get path and add slash if it isnt there
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            strcpy(dir, argv[i+1]);
            pathWithSlash(dir);
            arg = i+1;
        }
    }
    
    // create pipe
    if (pipe(p) < 0) {
        perror("Failed to open pipe\n");
        exit(1);
    }
    // create listener
    if ((pid_listener = fork()) < 0) {
        perror("Listener fork failed\n");
        exit(1);
    // listener writes
    } else if (pid_listener == 0) {
        listener_process = getpid();
        close(p[0]);
        // set pipe to stdout
        dup2(p[1], 1); 
        close(p[1]);
        // monitor create events
        if (execl("/usr/bin/inotifywait", "inotifywait", "-me", "create", dir, (char*)0) == -1) { 
            perror("Execl Failed\n");
            exit(1);
        }
    // manager reads  
    } else if (pid_listener > 0) { 
        printf("Watching directory %s\n", dir);
        close(p[1]);
        while(1) {
            // get filename
            nbytes = read(p[0], inbuf, BUFFER_SIZE);
            printf("%.*s", nbytes, inbuf);
            // remove newline from filename
            inbuf[strcspn(inbuf, "\n")] = 0;  
            // clean input
            getFilename((char*)&inbuf, &filename); 
            memset(fifo, 0, 100);
            strcpy(folder, "/tmp/");
            memset(path, 0, 100);
            memset(dir, 0, 100);
            strcpy(dir, argv[arg]);
            pathWithSlash(dir);
            strcpy(path, strcat(dir, strcat(filename, "\n")));
            
            // there is some available worker
            if (availableWorker() != -1) {
                printf("available\n");

                sprintf(mypid, "%d", availableWorker());
                strcpy(fifo, strcat(folder, mypid));

                // send signal to child to wake up
                if (kill(availableWorker(), SIGCONT) == -1) {
                    perror("Failed to continue worker\n");
                    exit(1);
                }
                if ((infile = open(fifo, O_WRONLY)) < 0) {
                    perror("Failed to open named pipe\n");
                    exit(1);
                }
                // write file path in named pipe
                if (write(infile, path, 100) < 0) {
                    perror("Failed to write in named pipe\n");
                    exit(1);
                }
                if (close(infile) < 0) {  
                    perror("Failed to close named pipe\n");
                    exit(1);
                }  
            } else { 
                // no available workers -> create one
                if ((pid_worker = fork()) < 0) {
                    perror("Worker fork failed\n");
                    exit(1);
                } else if (pid_worker == 0) {
                    worker_process = getpid();
                    printf("worker %d\n", worker_process);
                    // receive signal from parent to restart if stopped
                    signal(SIGCONT, SIG_DFL); 
                    // do my job
                    worker();
                    // child exits
                    exit(0); 
                } else if (pid_worker > 0) {
                    // get stopped child
                    signal(SIGCHLD, sigchld_handler);
                    printf("parent %d\n", getpid());
                    // push worker to the queue
                    push(pid_worker);
                    sprintf(mypid, "%d", pid_worker);
                    strcpy(fifo, strcat(folder, mypid));
                    // create named pipe
                    if (mkfifo(fifo, PERMS) < 0) {
                        perror("Can't create fifo\n");
                    }
                    if ((infile = open(fifo, O_WRONLY)) < 0) {
                        perror("Failed to open named pipe\n");
                        exit(1);
                    }
                    // write file path in named pipe
                    if (write(infile, path, 100) < 0) {
                        perror("Failed to write in named pipe\n");
                        exit(1);
                    }
                    if (close(infile) < 0) {  
                        perror("Failed to close named pipe\n");
                        exit(1);
                    }  
                }
            }
        }
    }
}

