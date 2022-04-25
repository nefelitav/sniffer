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
pid_t listener_process;
char dir[100];
int main(int argc, char **argv) {

    char inbuf[BUFFER_SIZE];
    int p[2], nbytes = 0, code, infile;
    pid_t pid_worker;
    char fifo[100], folder[10], path[100];   
    char* filename = NULL;
    char mypid[100];
    int arg = 0;

    createPidQueue();
    memset(path, 0, 100);
    memset(dir, 0, 100);
    memset(inbuf, 0, BUFFER_SIZE);

    signal(SIGINT, sigint_handler);

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            strcpy(dir, argv[i+1]);
            arg = i+1;
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
        listener_process = getpid();
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
            inbuf[strcspn(inbuf, "\n")] = 0;  // remove newline from filename
            getFilename((char*)&inbuf, &filename);
            memset(fifo, 0, 100);
            strcpy(folder, "/tmp/");
            memset(path, 0, 100);
            memset(dir, 0, 100);
            strcpy(dir, argv[arg]);
            strcpy(path, strcat(dir, strcat(filename, "\n")));
            
            if (availableWorker() != -1) {
                printf("available\n");

                sprintf(mypid, "%d", availableWorker());
                strcpy(fifo, strcat(folder, mypid));

                // send signal to child to wake up
                kill(availableWorker(), SIGCONT);
                if ((infile = open(fifo, O_WRONLY)) < 0) {
                    perror("File open failed\n");
                    exit(4);
                }
                write(infile, path, 100);
                if (close(infile) < 0) {  
                    perror("File close failed\n");
                    exit(6);
                }  
            } else { 
                // no available workers -> create one
                if ((pid_worker = fork()) < 0) {
                    perror("Fork Failed\n");
                    exit(2);
                } else if (pid_worker == 0) {
                    worker_process = getpid();
                    printf("worker %d\n", worker_process);
                    // receive signal from parent to restart if stopped
                    signal(SIGCONT, SIG_DFL); 
                    worker();
                    exit(0); // child exits
                } else if (pid_worker > 0) {
                    signal(SIGCHLD, sigchld_handler);
                    printf("parent %d\n", getpid());
                    push(pid_worker);
                    sprintf(mypid, "%d", pid_worker);
                    strcpy(fifo, strcat(folder, mypid));
                    if (mkfifo(fifo, PERMS) < 0) {
                        perror("Can't create fifo\n");
                    }
                    if ((infile = open(fifo, O_WRONLY)) < 0) {
                        perror("File open failed\n");
                        exit(4);
                    }
                    write(infile, path, 100);
                    if (close(infile) < 0) {  
                        perror("File close failed\n");
                        exit(6);
                    }  
                }
            }
        }
    }
    deletePidQueue();
    exit(0);
}

