#ifndef _ThreadSharedMemTypes_H
#define _ThreadSharedMemTypes_H

#include "../Libraries/frame.h"

typedef struct {
  int sockfd;
  Frame frame;
  int danny_num;
} ThreadData;

typedef struct {
char name[DATA_SIZE]; //We use a fixed size buffer for 2 reasons. 1. to not have to modify the shared memory region every time, and 2. because we know the max size of the name
float temperature;
int humidity;
float atmosphere_pressure;
float precipiataion;
} SharedMemData;
#endif
