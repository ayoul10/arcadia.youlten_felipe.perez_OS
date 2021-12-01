//
// Created by Felipe Perez (felipe.perez)
// and Arcadia Youlten (arcadia.youlten)
// on 17/10/2020.
//

#define _GNU_SOURCE
//own libraries
#include "../Libraries/semaphore.h"
#include "../Libraries/file_processor.h"
#include "WendyThreadManager.h"
#include "../Libraries/frame.h"
#include "../Libraries/ConnectionUtils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>  // For wait function
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <poll.h>
#include <sys/signal.h>
#include <sys/shm.h>
#include <pthread.h>

//defines
#define WAITING "Waiting...\n"
#define NEW_CONNECTION "New Connection: \n"
#define START_WENDY "Starting Wendy...\n"
#define DISCONNECT_WENDY "\nDisconnecting Wendy...\n\n"

//Global variables
ConfigGeneral config;

pthread_t tid_array[MAX_CONNECTIONS];
int num_dannys = 0;
int danny_sockets[MAX_CONNECTIONS];
int sockfd = 0;

static void *ThreadFunctionWendy (void *arg){
  ThreadData * aux = (ThreadData *) arg;
  DannyProcessWendy(aux);
  return NULL;
}


void ksighandler(int signum){
  int s = -1;
  int return_close = -1;
  char string[200];

  switch (signum){

    case SIGINT:

        sprintf(string, DISCONNECT_WENDY); //spritnf -- write formatted data to string
        write(1, string, strlen(string));

        for (int i = 0; i < num_dannys; i++) {

          return_close = close(danny_sockets[i]);

          if(return_close < 0){
            sprintf(string, "\nWe didn't close Danny number %d \n", i); //spritnf -- write formatted data to string
            write(STDOUT_FILENO, string, strlen(string));
          }

          sprintf(string, "\nClosed\n"); //spritnf -- write formatted data to string
          write(STDOUT_FILENO, string, strlen(string));

          s= pthread_join(tid_array[i], NULL);
           if (s != 0){
            sprintf(string, "Something's wrong couldn't join thread in wendy\n");
            write(STDOUT_FILENO, string, strlen(string));
          }
        }

        // close listening file descriptor
        close(sockfd);

        // FREES!
        free(config.ip);
        raise(SIGTERM);

      break;

    default:
      sprintf(string, "\n\nOther signal raised:%d\n\n", signum); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));
        break;
  }
  // reprogram the executed signal to perform the same
  signal(signum, ksighandler);
}


int main(int argc, char *argv[]){

  //variables needed for fork and pre-fork
  char string[200];

  //fork to create Lloyd process as child
  //  if child, call lloyd process function !till ded!
  signal(SIGINT, ksighandler);
  signal(SIGALRM, ksighandler);

  int fd_file_config;
  Frame frame;

  // check if the user actually input a parameter
  if(argc > 1){
    // reading the config files
    fd_file_config = open(argv[argc-1], O_RDONLY, 0600);

    if(fd_file_config == -1){
      sprintf(string, "Unable to open config file!\n");
      write(STDOUT_FILENO, string, strlen(string));
    }else {

      // save config
      loadGeneralConfig(fd_file_config, &config);
      close(fd_file_config);

      sprintf(string, "Wendy's IP: %s\n", config.ip);
      write(STDOUT_FILENO, string, strlen(string));

      sprintf(string, "Wendy's port: %d\n", config.port);
      write(STDOUT_FILENO, string, strlen(string));

      sockfd = setUpConnection(config, 0);

      //Tell the OS we want to reuse the Jack socket
      int one = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0){
        sprintf(string, "setsockopt Failed\n");
        write(STDOUT_FILENO, string, strlen(string));
      }

      sprintf(string, "\nWaiting\n"); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));

      listen(sockfd, MAX_CONNECTIONS);

      ThreadData thread_data;

      while(1){
        int newsock = acceptDannyConnections(sockfd, &frame);
        if(newsock != -1){
          // start a danny thread with the new socket,
          danny_sockets[num_dannys] = newsock;
          thread_data.sockfd = newsock;
          thread_data.frame = frame;
          thread_data.danny_num = num_dannys;
          thread_data.unbound_fd = sockfd;

          pthread_create(&tid_array[num_dannys], NULL, ThreadFunctionWendy, &thread_data);
          num_dannys++;
        }
      }
    }
  }else{
    sprintf(string, "Please input a file as a parameter\n");
    write(STDOUT_FILENO, string, strlen(string));
  }
  return(0);
}
