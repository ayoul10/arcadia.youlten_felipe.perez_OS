CC=gcc
CFLAGS= -I -g -Wall -Wextra -ggdb3 -lpthread
DEPS = ../Libraries/file_processor.h ../Libraries/file_processor.c WendyThreadManager.c WendyThreadManager.h ../Libraries/frame.c ../Libraries/frame.h ../Libraries/ConnectionUtils.h ../Libraries/ConnectionUtils.c
OBJ = main.o ../Libraries/file_processor.o WendyThreadManager.o ../Libraries/frame.o ../Libraries/ConnectionUtils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Wendy: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	-rm edit $(objects)
