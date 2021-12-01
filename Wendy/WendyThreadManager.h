#ifndef _WendyThreadManager_H
#define _WendyThreadManager_H

#define POLLRDHUP	0x2000
#define BARRY_DIR "Barry/"

#include "../Libraries/file_processor.h"
#include "../Libraries/frame.h"
#include "../Libraries/ConnectionUtils.h"
#include "../Libraries/semaphore.h"
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
#include <sys/wait.h>
#include <fcntl.h>
#include <poll.h>

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)


typedef struct {
  int sockfd;
  Frame frame;
  int danny_num;
  int unbound_fd;
} ThreadData;

typedef struct {
  unsigned char md5sum[33];
  char * file_name;
  char * file_size;
  unsigned char * image_data;
} Image;

void DannyProcessWendy (ThreadData *aux);
#endif
