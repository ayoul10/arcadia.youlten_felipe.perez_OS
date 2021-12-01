//
// Created by Felipe Perez (felipe.perez)
// and Arcadia Youlten (arcadia.youlten)
// on 17/10/2020.
//

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/wait.h>  // For wait function
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <poll.h>

//own libraries
#include "../Libraries/file_processor.h"
#include "DannyConnections.h"
#include "../Libraries/frame.h"
#include "GeneratePrint.h"

//declares
#define TESTING "Testing...\n"
#define NO_FILES_AVALIABLE "No files avaliable\n"
#define FILES_FOUND " files found\n"
#define START_DANNY "Starting Danny...\n"
#define DISCONNECT_DANNY "\nDisconnecting Danny...\n\n"

//Global variables
char string[200];
char data_buffer[DATA_SIZE];
ConfigDanny config;
int sockfd_jack = -1;
int sockfd_wendy = -1;
struct pollfd poll_fd_jack;
struct pollfd poll_fd_wendy;

void freeConfig(ConfigDanny config){
  free(config.weather_station);
  free(config.file_directory);
  free(config.ip_jack);
  free(config.ip_wendy);
}


void ksighandler(int signum){

  int read_size =0;
  int num_files = 0;

  Frame frame;
  char formatted_data[DATA_SIZE];
  char aux_frame[FRAME_SIZE];
  unsigned long long bytes_sent =0;
  unsigned long long bytes_to_send =0;

    switch (signum){

    case SIGINT:
        sprintf(string, DISCONNECT_DANNY); //spritnf -- write formatted data to string
        write(1, string, strlen(string));
        //potential to disconect any active connections in the future after this signal is recieved
        bzero(data_buffer, DATA_SIZE);
        strcpy(data_buffer, config.weather_station);
        //Send disconnection FRAME if we are still connected
        if(sockfd_jack > 0)
          fillAndSendFrame(sockfd_jack, SOURCE_DANNY, TYPE_DISCONNECT, data_buffer, "\0", 0);

        if(sockfd_wendy > 0)
          fillAndSendFrame(sockfd_wendy, SOURCE_DANNY, TYPE_DISCONNECT, data_buffer, "\0", 0);

        //time for the disconect frame to be processed in Jack and Wendy
        sleep(1);

        //FREES!!
        freeConfig(config);

        //close the sockets
        if(sockfd_jack > 0)
          close(sockfd_jack);
        if(sockfd_wendy > 0)
          close(sockfd_wendy);

        raise(SIGTERM);

        break;

    case SIGALRM:

        sprintf(string, TESTING);
        write(STDOUT_FILENO, string, strlen(string));

        //prepare to poll the Jack socket
        poll_fd_jack.fd = sockfd_jack;
        poll_fd_jack.events = POLLNVAL | POLLRDHUP | POLLIN;
        poll(&poll_fd_jack, 1, 0);

        //prepare to poll the Wendy socket
        poll_fd_wendy.fd = sockfd_wendy;
        poll_fd_wendy.events = POLLNVAL | POLLRDHUP | POLLIN;
        poll(&poll_fd_wendy, 1, 0);

        if((poll_fd_jack.revents & (POLLRDHUP | POLLNVAL)) || (poll_fd_wendy.revents & (POLLRDHUP | POLLNVAL))){

          sprintf(string, "Socket closed\n");
          write(STDOUT_FILENO, string, strlen(string));

          raise(SIGINT);
        }else{

          // PRINT ALL FILES IN DIRECTORY
          printNumFilesAndNames(config);

          WeatherFormat * data = NULL;
          // READ FILES IN THE INDICATED DIRECTORY, CHECK FOR TXT FILES
          data = checkTxtFiles(config, &num_files);

          for (int i = 0; i < num_files; i++) {
            printWeatherStationInfo(data[i]);

            sprintf(string, "Sending data\n");
            write(STDOUT_FILENO, string, strlen(string));

            generateWeatherString(data[i], formatted_data);

            fillAndSendFrame(sockfd_jack, SOURCE_DANNY, TYPE_DATA, formatted_data, "\0", 0);

            read_size = read(sockfd_jack, aux_frame, FRAME_SIZE);

            if(getFrameData(&frame, aux_frame, read_size)){
              // format wrong
              sprintf(string, "Error: Frame format incorrect\n");
              write(STDOUT_FILENO, string, strlen(string));

              // checking frame data
              // (if source is jack, and frame type is connection)
            }else if((strcmp(frame.source, SOURCE_JACK) != 0) && frame.type != TYPE_OK_DATA){
              sprintf(string, "Data sent\n");
              write(STDOUT_FILENO, string, strlen(string));
            }
          }

          for (int i = 0; i < num_files; i++) {
            free(data[i].file_name);
            free(data[i].date);
            free(data[i].hour);
            free(data[i].temperature);
            free(data[i].humidity);
            free(data[i].atmosphere_pressure);
            free(data[i].precipiataion);
          }

          free(data);
          data = NULL;

          // NEXT, CHECK FOR JPG FILES
          num_files=0;
          ImageFormat * images = checkJPGFiles(config, &num_files, sockfd_jack, sockfd_wendy);

          for (int i = 0; i < num_files; i++) {
            sprintf(string, "Sending %s\n", images[i].file_name);
            write(STDOUT_FILENO, string, strlen(string));

            generateImageString(images[i], formatted_data);

            fillAndSendFrame(sockfd_wendy, SOURCE_DANNY, TYPE_IMAGE, formatted_data, "\0", 0);

            bytes_to_send = strtoull(images[i].size_in_bytes, NULL, 10);
            bytes_sent = 0;

            while(1){

              if(bytes_to_send > DATA_SIZE){
                for (unsigned long long j = 0; j < DATA_SIZE; j++) {
                  formatted_data[j] = images[i].image_data[j+bytes_sent];
                  //sprintf(string, "|%X|", images[i].image_data[j+bytes_sent]);
                  //write(STDOUT_FILENO, string, strlen(string));
                }

                //sprintf(string, "\n");
                //write(STDOUT_FILENO, string, strlen(string));

                fillAndSendFrame(sockfd_wendy, SOURCE_DANNY, TYPE_IMAGE_FRAME, formatted_data, "\0", 0);

                bytes_sent = bytes_sent + DATA_SIZE;
                bytes_to_send = bytes_to_send - DATA_SIZE;
              }else{
                //send all remaining info
                for (unsigned long long j = 0; j < bytes_to_send; j++) {
                  formatted_data[j] = images[i].image_data[j+bytes_sent];
                  //sprintf(string, "|%X|", images[i].image_data[j+bytes_sent]);
                  //write(STDOUT_FILENO, string, strlen(string));
                }
                
                fillAndSendFrame(sockfd_wendy, SOURCE_DANNY, TYPE_IMAGE_FRAME, formatted_data, "\0", 0);

                break;
              }
            }

            read_size = read(sockfd_wendy, aux_frame, FRAME_SIZE);

            if(getFrameData(&frame, aux_frame, read_size)){
              // format wrong
              sprintf(string, "Error: Frame format incorrect\n");
              write(STDOUT_FILENO, string, strlen(string));

              // checking frame data
              // (if source is wendy, and frame type is connection)
            }else if((strcmp(frame.source, SOURCE_WENDY) == 0) && frame.type == TYPE_OK_IMAGE){
              sprintf(string, "%s\n", frame.data);
              write(STDOUT_FILENO, string, strlen(string));
            }else if((strcmp(frame.source, SOURCE_WENDY) == 0) && frame.type == TYPE_ERROR_IMAGE){
              sprintf(string, "%s\n", frame.data);
              write(STDOUT_FILENO, string, strlen(string));
            }else{
              sprintf(string, "Error: Frame Error\n");
              write(STDOUT_FILENO, string, strlen(string));
            }
            //sleep(5);
          }

          for (int i = 0; i < num_files; i++) {
            free(images[i].image_data);
            free(images[i].file_name);
            free(images[i].size_in_bytes);
          }
          free(images);
        }

        //reprogram the alarm
        alarm(config.time_to_wait);
        break;

    default:
      sprintf(string, "\n\nOther signal raised:%d\n\n", signum); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));
        break;
    }
    /* reprogram the executed signal to perform the same */
    signal(signum, ksighandler);
}

int main(int argc, char *argv[]){
    //local variables
    int fd_file_config;

    //check if the user actually input a parameter

    if(argc > 1){
      //reading the config files
      fd_file_config = open(argv[argc-1], O_RDONLY, 0600);

      if(fd_file_config == -1){
          sprintf(string, "Unable to open config file!\n");
          write(STDOUT_FILENO, string, strlen(string));
      }else {
        //save config
        loadDannyConfig(fd_file_config, &config);

        close(fd_file_config);

        printConfigInfo(config);

        signal(SIGINT, ksighandler);
        signal(SIGALRM, ksighandler);
        //signal(SIGUSR1, ksighandler);

        // Connect to Jack
        sockfd_jack = connectToJackWendy(config.ip_jack, config.port_jack, config.weather_station);
        if(sockfd_jack < 0){
          sprintf(string, "Connection Failed Jack\n");
          write(STDOUT_FILENO, string, strlen(string));
          raise(SIGINT);
        }

        // Connect to Wendy
        sockfd_wendy = connectToJackWendy(config.ip_wendy, config.port_wendy, config.weather_station);
        if(sockfd_wendy < 0){
          sprintf(string, "Connection Failed Wendy\n");
          write(STDOUT_FILENO, string, strlen(string));
          raise(SIGINT);
        }

        //Tell the OS we want to reuse the Jack socket
        int one = 1;
        if (setsockopt(sockfd_jack, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0 || setsockopt(sockfd_wendy, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0){
          sprintf(string, "setsockopt Failed\n");
          write(STDOUT_FILENO, string, strlen(string));
        }

        alarm(config.time_to_wait);

        while (1) {
          pause();
        }

      }
    }else{
      sprintf(string, "Please input a file as a parameter\n");
      write(STDOUT_FILENO, string, strlen(string));
    }

    return(0);
}
