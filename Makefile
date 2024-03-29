OBJS = manager.o worker.o utilities.o
OUT = sniffer
CC = gcc
FLAGS = -g -c -Wall

all : $(OBJS)
	rm -f /tmp/*.out 
	$(CC) -g -Wall -o $(OUT) $(OBJS)

manager.o : ./src/manager.c
	$(CC) $(FLAGS) ./src/manager.c

worker.o : ./src/worker.c
	$(CC) $(FLAGS) ./src/worker.c

utilities.o : ./src/utilities.c
	$(CC) $(FLAGS) ./src/utilities.c

valgrind: $(OUT)
	rm -f /tmp/*.out 
	valgrind --leak-check=full --show-leak-kinds=all  --track-origins=yes ./sniffer

clean :
	rm -f /tmp/*.out 
	rm -f $(OBJS) $(OUT)