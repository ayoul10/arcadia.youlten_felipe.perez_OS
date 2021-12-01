#ifndef _JackThreadManager_H
#define _JackThreadManager_H

#define POLLRDHUP	0x2000
#include "../Libraries/semaphore.h"
#include "ThreadSharedMemTypes.h"
#include "../Libraries/file_processor.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>

SharedMemData processDannyWeatherinfo(Frame frame, char * station_name);

void DannyProcessJack (ThreadData *aux, SharedMemData * sh_data, semaphore sem_sync_lloyd, semaphore sem_lloyd_done, pthread_mutex_t sem_mutex_shmem);

#endif
