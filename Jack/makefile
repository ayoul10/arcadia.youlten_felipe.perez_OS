CC=gcc
CFLAGS= -I -g -Wall -Wextra -ggdb3 -lpthread
DEPS = ../Libraries/file_processor.h ../Libraries/file_processor.c JackThreadManager.c JackThreadManager.h ../Libraries/frame.c ../Libraries/frame.h ../Libraries/ConnectionUtils.h ../Libraries/ConnectionUtils.c Lloyd.h Lloyd.c
OBJ = main.o ../Libraries/file_processor.o JackThreadManager.o ../Libraries/frame.o ../Libraries/ConnectionUtils.o Lloyd.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Jack: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	-rm edit $(objects)
