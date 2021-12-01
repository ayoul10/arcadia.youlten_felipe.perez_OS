#include "DannyConnections.h"

int connectToJackWendy (char * ip, int port, char * weather_station){
  char aux_frame[FRAME_SIZE];
  char string[150];
  char data_buffer[DATA_SIZE];
  int read_size=0;

  Frame frame;
  ConfigGeneral config;
  config.ip = ip;
  config.port = port;

  int sockfd = setUpConnection(config, 1);

  // We can connect to the server casting the struct:
  // bind waits for a struct sockaddr * and we are passing struct sockaddr_in *

  // send connection request frame
  bzero(data_buffer, DATA_SIZE);
  strcpy(data_buffer, weather_station);
  fillAndSendFrame(sockfd, SOURCE_DANNY, TYPE_CONNECT, data_buffer, "\0", 0);

  read_size = read(sockfd, aux_frame, FRAME_SIZE);

  // checking reply frame format while at the same
  // time getting the frame data if successful
  if(getFrameData(&frame, aux_frame, read_size)){
    //format wrong
    sprintf(string, "Error: Frame format incorrect\n");
    write(STDOUT_FILENO, string, strlen(string));

    return -1;

  // checking frame data
  // (if source is jack or wendy, and frame type is connection)
  }else if((checkSource(frame.source, SOURCE_JACK) || checkSource(frame.source, SOURCE_WENDY)) && !checkType(frame.type,TYPE_CONNECT)){
    sprintf(string, "%s\n", frame.data);
    write(STDOUT_FILENO, string, strlen(string));

    return -1;

  // all OK
  }else{
    return sockfd;
  }

}
