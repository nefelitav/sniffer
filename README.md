# Sniffer

[![C CI Actions Status](https://github.com/nefelitav/sniffer/workflows/C%20CI/badge.svg)](https://github.com/nefelitav/sniffer/actions)

This program consists of 3 entities:

- Manager : the main process that communicates with the listener through a pipe and with the workers through named pipes. All these processes are created using fork and are terminated pressing CTRL+C (SIGINT) by the parent/manager. For every new file that is created in the folder that the listener monitors, a new worker is created, if there is no one available, and this worker is responsible of processing the specific file. As far as the named pipes are concerned, they are stored in the /tmp directory and each is named after the worker's process id. The manager writes the name of the file that should be processed inside the named pipe. Also, some other functionality of the manager is that it creates a queue containing the workers' process ids and their availability, which it handles with singals. That means that when the manager receives a SIGCHLD signal, it finds the stopped worker and sets it to available and also when the manager sends a SIGCONT signal to the worker, in order to continue with some new file, it sets it to unavailable.

- Listener : informs manager of newly created files in a specific folder, using exec and the linux command inotifywait, which is found in the /usr/bin directory. The arguments used for this command are -m, for monitoring and -e create, so that it focuses on creation of files.

- Workers : They read the name of the file that they are going to process from the named pipe, then they open that file and search for urls, using low level i/o. Using lseek they find the size of the file and using read, strstr and strtok_r they find some possible urls, which are then filtered with the regex library for validation. If a url is valid, then the worker gets the location out of it, removing extra data, and then it is added in a linked list which stores each location's number of occurences in this file. Then, traversing this list, the worker writes all the data in a new .out file, which is stored also in the /tmp directory and which is removed on make/make clean/make valgrind. The worker's job is done and so it sends a SIGSTOP to itself and waits to continue whenever the manager tells them to do so.

File structure:

- `manager.c` : contains the manager and listener functionality, as well as the main structure of the whole program.
- `worker.c` : contains the worker functionality, as well as the urlsList struct methods, which are used by workers.
- `utilities.c` : contains some useful functions (e.g for path processing), signal handlers and the pidsQueue struct methods.

Finally, I created a bash script to search for the occurence of some given TLD in all the .out files that were created in the /tmp directory.

All in all, in this project I familiarized with the following concepts:

- Interprocess Communication
- Signals
- Pipes, named pipes
- Error Handling
- Low level I/O
- Shell scripting

Compile & Run

```
$ make && ./sniffer [-p path]
```

Check for memory leaks

```
$ make valgrind [-p path]
```

Run bash script

```
$ ./finder.sh [one or more TLD (e.g. com org gr)]
```
