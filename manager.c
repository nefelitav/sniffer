#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#define PERMS 0666
#define BUFFER_SIZE 4096

int main(int argc, char **argv) {

    char inbuf[BUFFER_SIZE];
    int p[2], nbytes = 0, code, status;
    pid_t pid_listener, pid_worker;
    char dir[100];

    // char * myfifo = "/tmp/myfifo";
    // if (mkfifo(myfifo, PERMS) < 0) {
    //     perror("can't create fifo");
    // }

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
    } else if (pid_listener == 0) { // worker writes
        close(p[0]);
        dup2(p[1], 1); // set pipe to stdout
        close(p[1]);
        code = execl("/usr/bin/inotifywait", "inotifywait", "", dir, (char*)0); //--monitor
        if (code == -1) {
            perror("Execl Failed\n");
        }
    } else if (pid_listener > 0) { // manager reads 
        printf("Watching directory %s\n", dir);
        close(p[1]);
        //while(1) {
            nbytes = read(p[0], inbuf, BUFFER_SIZE);
            printf("%.*s\n", nbytes, inbuf);
            // if ((pid_worker = fork()) < 0) {
            //     perror("Fork Failed\n");
            //     exit(2);
            // } else if (pid_worker == 0) {
                
            // } 
        //}

        // do {
        //     if (waitpid(pid_listener, &status, WUNTRACED | WCONTINUED) == -1) { //wait for child to finish
        //         return -1;
        //     }
        // } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        wait(NULL);
    }
    exit(0);
}