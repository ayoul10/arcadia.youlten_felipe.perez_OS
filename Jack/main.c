//
// Created by Felipe Perez (felipe.perez)
// and Arcadia Youlten (arcadia.youlten)
// on 17/10/2020.
//

//ports
#define _GNU_SOURCE
//own libraries
#include "../Libraries/semaphore.h"
#include "../Libraries/file_processor.h"
#include "JackThreadManager.h"
#include "../Libraries/frame.h"
#include "ThreadSharedMemTypes.h"
#include "../Libraries/ConnectionUtils.h"
#include "Lloyd.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
#define GET_DATA " files found\n"
#define START_JACK "Starting Jack...\n"
#define DISCONNECT_JACK "\nDisconnecting Jack...\n\n"

//Global variables
ConfigGeneral config;

pthread_t tid_array[MAX_CONNECTIONS];
int num_dannys = 0;
int danny_sockets[MAX_CONNECTIONS];
int sockfd = 0;
int ppid = 0;
int lloydPid = -1;
int lloyd_write = 0;

//shmem variables
SharedMemData *sh_data;
int memid;

semaphore sem_sync_lloyd;
pthread_mutex_t sem_mutex_shmem = PTHREAD_MUTEX_INITIALIZER; //make this an actual mutex type variable so that it's thread safe
semaphore sem_lloyd_done;

static void *ThreadFunctionJack (void *arg){
  ThreadData * aux = (ThreadData *) arg;
  DannyProcessJack(aux, sh_data, sem_sync_lloyd, sem_lloyd_done, sem_mutex_shmem);
  return NULL;
}

void ksighandler(int signum){
  int s = -1;
  int return_close = -1;
  char string[200];

  switch (signum){

    case SIGINT:
      // parent
      if(ppid == getpid()){

        sprintf(string, DISCONNECT_JACK); //spritnf -- write formatted data to string
        write(1, string, strlen(string));

        //send sigint to lloyd so that it commit die
        kill(lloydPid, SIGINT);

        // wait till Lloyd ded
        wait(NULL);

        // close semaphores.
        pthread_mutex_destroy(&sem_mutex_shmem);
        SEM_destructor(&sem_sync_lloyd);
        SEM_destructor(&sem_lloyd_done);

        // unlink and delete shared memory
        shmdt(sh_data);
        shmctl(memid, IPC_RMID, NULL);

        for (int i = 0; i < num_dannys; i++) {

          return_close = close(danny_sockets[i]);

          if(return_close < 0){
            sprintf(string, "\nDanny number %d was already closed\n", i); //spritnf -- write formatted data to string
            write(STDOUT_FILENO, string, strlen(string));
          }

          sprintf(string, "\nClosed\n"); //spritnf -- write formatted data to string
          write(STDOUT_FILENO, string, strlen(string));

          s= pthread_join(tid_array[i], NULL);
           if (s != 0){
            sprintf(string, "Something's wrong, can't join from a thread \n");
            write(STDOUT_FILENO, string, strlen(string));
          }
        }

        // close listening file descriptor
        close(sockfd);

        // FREES!
        free(config.ip);
        raise(SIGTERM);
      }else{
        sh_data->name[0]='\0';
        SEM_signal(&sem_sync_lloyd);
        //maybe this will be a problem for the 3 semaphores implementation, to be kept in mind
      }

      break;

    case SIGALRM:
      // tell Lloyd to write to the file
      strcpy(sh_data->name, LLOYD_WRITE);
      alarm(LLOYD_SECONDS);
      break;

    default:
      sprintf(string, "\n\nOther signal raised:%d\n\n", signum);
      write(STDOUT_FILENO, string, strlen(string));
        break;
  }
  // reprogram the executed signal to perform the same
  signal(signum, ksighandler);
}


int main(int argc, char *argv[]){

  //variables needed for fork and pre-fork
  char string[200];
  ppid = getpid();
  //initialize semapfores

  SEM_constructor(&sem_sync_lloyd);
  SEM_init(&sem_sync_lloyd, 0);

  SEM_constructor(&sem_lloyd_done);
  SEM_init(&sem_lloyd_done, 0);

  //set up sharedMemoryRegion
  //key is IPC_PRIVATE
  memid = shmget(IPC_PRIVATE, sizeof(SharedMemData), IPC_CREAT | IPC_EXCL | 0600 );

  if (memid < 0){
    sprintf(string, "Shared memory problem\n");
    write(STDOUT_FILENO, string, strlen(string));
    exit(EXIT_FAILURE);
  }else{
    //link shmem
    sh_data = shmat(memid, 0, 0);

    //fork to create Lloyd process as child
    //  if child, call lloyd process function !till ded!
    signal(SIGINT, ksighandler);
    //program alarm

    // fork
    lloydPid = fork();

    ////////////////////////////////////////////////////////LLOYD PROCESS
    if (lloydPid == 0) { // CHILD = LLOYD
      signal(SIGALRM, ksighandler);
      alarm(LLOYD_SECONDS);
      lloydProcess(sh_data, sem_sync_lloyd, sem_lloyd_done);
    }

    //if parent, continue

    ////////////////////////////////////////////////////////JACK PROCESS
    //jack variables
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

        sprintf(string, "Jack's IP: %s\n", config.ip);
        write(STDOUT_FILENO, string, strlen(string));

        sprintf(string, "Jack's port: %d\n", config.port);
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

            pthread_create(&tid_array[num_dannys], NULL, ThreadFunctionJack, &thread_data);
            num_dannys++;
          }
        }
      }
    }else{
      sprintf(string, "Please input a file as a parameter\n");
      write(STDOUT_FILENO, string, strlen(string));
    }
  }
  return(0);
}
