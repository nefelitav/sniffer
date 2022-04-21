OBJS = manager.o
SOURCE = manager.c
OUT = sniffer
CC = gcc
FLAGS = -g -c -Wall

all : $(OBJS)
	$(CC) -g -Wall -o $(OUT) $(OBJS)

manager.o : manager.c
	$(CC) $(FLAGS) manager.c

valgrind: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all  --track-origins=yes ./$(OUT)

helgrind: $(OUT)
	valgrind --tool=helgrind ./$(OUT)

clean :
	rm -f $(OBJS) $(OUT)