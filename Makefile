OBJS = manager.o worker.o utilities.o
SOURCE = manager.c worker.c utilities.c
OUT = sniffer
CC = gcc
FLAGS = -g -c -Wall

all : $(OBJS)
	$(CC) -g -Wall -o $(OUT) $(OBJS)

manager.o : manager.c
	$(CC) $(FLAGS) manager.c

worker.o : worker.c
	$(CC) $(FLAGS) worker.c

utilities.o : utilities.c
	$(CC) $(FLAGS) utilities.c

valgrind: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all  --track-origins=yes ./$(OUT) -p /mnt/c/Users/ntavoula/Desktop/tmp/

clean :
	rm -f $(OBJS) $(OUT)