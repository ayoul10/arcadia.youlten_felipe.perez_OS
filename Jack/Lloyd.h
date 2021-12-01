#ifndef _Lloyd_H
#define _Lloyd_H

#define _GNU_SOURCE
#include "../Libraries/semaphore.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include "ThreadSharedMemTypes.h"
#include "../Libraries/ConnectionUtils.h"

#define LLOYD_SECONDS 10
#define LLOYD_WRITE "?-?"

typedef struct {
  char name[DATA_SIZE];
  int num_times;
  float avg_temperature;
  int avg_humidity;
  float avg_atmosphere_pressure;
  float avg_precipiataion;
} LloydData;

void lloydProcess(SharedMemData * sh_data, semaphore sem_sync_lloyd, semaphore sem_lloyd_done);

#endif
