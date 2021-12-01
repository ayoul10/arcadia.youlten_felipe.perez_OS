#include "frame.h"

void fillFrame(char * source, char type, char data[DATA_SIZE], char * frame){

  char aux_source[SOURCE_SIZE];

  strcpy(aux_source, source);
  //fill part that isnt source data with \0
  for (int i = strlen(source); i < SOURCE_SIZE; i++) {
    aux_source[i] = '\0';
  }

  int index=0;
  for (int i = 0; i < FRAME_SIZE; i++) {

    if(i < SOURCE_SIZE){
      frame[i] = aux_source[index];
      index++;

    }else if(i == SOURCE_SIZE){
      frame[i] = type;
      index=0;

    }else{
      frame[i] = data[index];
      index++;
    }
  }
}

void sendFrame(int sockfd, char * frame){
  write(sockfd, frame, FRAME_SIZE);
}

void readFrame(int sockfd, char * frame){
  char aux_frame[115];

  read(sockfd, aux_frame, FRAME_SIZE+1);

  strcpy(frame, aux_frame);
}

int getFrameData(Frame * frame, char * aux_frame, int read_size){

  //char buffer_str[255];
  if (read_size == 115) {

    for (int i = 0; i < SOURCE_SIZE; i++) {
      frame->source[i] = aux_frame[i];
    }

    frame->type = aux_frame[SOURCE_SIZE];

    int index=0;
    for (int i = SOURCE_SIZE+1; i < FRAME_SIZE; i++) {
      frame->data[index] = aux_frame[i];
      index++;
    }

    return 0;

  }else{
    return 1;
  }
}

void disconnectFrame(int sockfd, char * station_name){
  char string[200];
  sprintf(string, "Closing socket from thread...\n");
  write(STDOUT_FILENO, string, strlen(string));
  close(sockfd);

  sprintf(string, "Closed\n");
  write(STDOUT_FILENO, string, strlen(string));

  free(station_name);
}

int weatherDataOK(char data[DATA_SIZE] ){

  int counter_hashtags;
  for (int i = 0; i < DATA_SIZE; i++) {
    if(data[i] == '#')
    counter_hashtags++;
  }
  return NUM_HASHTAGS_DATA == counter_hashtags;
}

void fillAndSendFrame(int sockfd, char * source, char type, char * data, char * print_message, int print_or_no){
  char aux_frame[115];
  char string[115];

  if(print_or_no){
    sprintf(string, "%s\n", print_message);
    write(STDOUT_FILENO, string, strlen(string));
  }

  // send back frame
  fillFrame(source, type, data, aux_frame);
  sendFrame(sockfd, aux_frame);
}

int checkSource(char * source_to_check, char * source_desired){
  if(strcmp(source_to_check, source_desired)){
    return 0;
  }else{
    return 1;
  }
}

int checkType(char type_to_check, char type_desired){
  return type_to_check == type_desired;
}
