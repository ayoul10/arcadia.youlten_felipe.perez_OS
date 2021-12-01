#include "ConnectionUtils.h"

uint16_t checkPortRange(int port_to_check){
    char buffer_str[255];
  if (port_to_check < 8859 || port_to_check  > 8880){
      sprintf(buffer_str, "Error: For this project, only ports between 8860 and 8879 are allowed (inclusive)\n");
      write(STDOUT_FILENO, buffer_str, strlen(buffer_str));
      raise(SIGINT);
  }
  return port_to_check;
}

//check IP address

struct in_addr checkIpAddress(char * ip_to_check){
  struct in_addr ip_addr;
  char buffer_str[255];
  if (inet_aton (ip_to_check, &ip_addr) == 0){
      sprintf (buffer_str, "inet_aton (%s): Failed\n", ip_to_check);
      write(STDOUT_FILENO, buffer_str, strlen(buffer_str));
      raise(SIGINT);
  }
  return ip_addr;
}
void checkSocket(int sockfd){
  char buffer_str[255];
  if (sockfd < 0) {
    sprintf(buffer_str, "Error: couldn't get socket fd\n");
    write(STDOUT_FILENO, buffer_str, strlen(buffer_str));
    raise(SIGINT);
  }

}

struct sockaddr_in fillOutSockAddrStruct(struct in_addr ip_addr, int port){

  struct sockaddr_in s_addr;
  bzero (&s_addr, sizeof (&s_addr));
  s_addr.sin_family = AF_INET;
  s_addr.sin_port = htons (port);
  s_addr.sin_addr = ip_addr;

  return s_addr;
}

int setUpConnection(ConfigGeneral config, int danny){
  char string[255];
  int sockfd = -1;
  // Check if the port is valid
  uint16_t port = checkPortRange(config.port);
  struct in_addr ip_addr = checkIpAddress(config.ip);

  // Create the socket
  sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
  // Check the socket
  checkSocket(sockfd);

  //struct that configures socket parameters
  struct sockaddr_in s_addr = fillOutSockAddrStruct(ip_addr, port);

  /*
  int size = 20000;

  if (0 != setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,  &size, sizeof(int))) {
      printf("Unable to set send buffer size, continuing with default size\n");
  }

  if (0 != setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,  &size, sizeof(int))) {
      printf("Unable to set receive buffer size, continuing with default size\n");
  }
  */

  //Bind if not danny, because danny wants to connect, not receive connections
  if(!danny){
    /*      BIND      */
    if (bind (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0){
      // print error
      sprintf(string, "Bind Error\n");
      write(STDOUT_FILENO, string, strlen(string));

      close(sockfd);
      raise(SIGINT);
    }
  }else{
    if (connect (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0){
      sprintf(string, "Error Connecting!\n");
      write(STDOUT_FILENO, string, strlen(string));
      close(sockfd);
      raise(SIGINT);
    }
  }

  return sockfd;

}

int acceptDannyConnections(int sockfd, Frame * frame){

  int read_size;
  char aux_frame[115];
  char data_buffer[DATA_SIZE];
  char string[250];

  // create new struct to fill with new client information that will be filled by accept function
  struct sockaddr_in c_addr;
  socklen_t c_len = sizeof (c_addr);

  // When executing accept we should add the same cast used in the bind function
  int newsock = accept (sockfd, (void *) &c_addr, &c_len);
  if (newsock < 0) {
    sprintf(string, "Accept error\n");
    write(STDOUT_FILENO, string, strlen(string));
    raise(SIGINT);
    exit (EXIT_FAILURE);
  }

  read_size = read(newsock, aux_frame, FRAME_SIZE);

  // checking frame format while at the same
  // time getting the frame data if successful
  bzero(data_buffer, DATA_SIZE);
  if(getFrameData(frame, aux_frame, read_size)){
    // send back an error frame frame and print Frame Error
    strcpy(data_buffer, DATA_FRAME_ERROR);
    fillAndSendFrame(newsock, SOURCE_JACK, TYPE_ERROR_FRAME, data_buffer, "Frame Error", 1);
    close(newsock);
    return -1;

  }else if(checkSource(frame->source, SOURCE_DANNY) && !(checkType(frame->type, TYPE_CONNECT))){
    // send back an connection error frame and print Frame Error
    strcpy(data_buffer, DATA_ERROR);
    fillAndSendFrame(newsock, SOURCE_JACK, TYPE_ERROR_CONNECT, data_buffer, "Connection Error", 1);
    close(newsock);
    return -1;
  }else{
    strcpy(data_buffer, DATA_CONNECT_OK);
    fillAndSendFrame(newsock, SOURCE_JACK, TYPE_CONNECT, data_buffer, "\0", 0);
    return newsock;
  }
}
