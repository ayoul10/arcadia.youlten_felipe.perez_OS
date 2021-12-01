#ifndef _ConnectionUtils_H
#define _ConnectionUtils_H

#define _GNU_SOURCE

//Includes
#include "../Libraries/frame.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#include "file_processor.h"

#define MAX_CONNECTIONS 10

int setUpConnection(ConfigGeneral config, int danny);

int acceptDannyConnections(int sockfd, Frame * frame);

#endif
