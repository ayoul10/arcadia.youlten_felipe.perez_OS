CC=gcc
CFLAGS= -I -g -Wall -Wextra -ggdb3
DEPS = ../Libraries/Connectionutils.c ../Libraries/Connectionutils.h ../Libraries/file_processor.h ../Libraries/file_directory.c DannyConnections.h DannyConnections.c ../Libraries/frame.c ../Libraries/frame.h GeneratePrint.c GeneratePrint.h
OBJ = main.o ../Libraries/file_processor.o DannyConnections.o ../Libraries/frame.o ../Libraries/ConnectionUtils.o GeneratePrint.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

Danny: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	-rm edit $(objects)
