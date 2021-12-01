#ifndef _DannyConnections_H
#define _DannyConnections_H

#define _GNU_SOURCE

#include "../Libraries/file_processor.h"
#include "../Libraries/ConnectionUtils.h"
#include "../Libraries/frame.h"
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
#include <signal.h>

int connectToJackWendy(char * ip, int port, char * weather_station);

#endif
