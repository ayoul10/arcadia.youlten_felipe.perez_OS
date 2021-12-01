#ifndef _frame_H
#define _frame_H

#define _GNU_SOURCE
#define SOURCE_SIZE 14
#define DATA_SIZE 100
#define FRAME_SIZE 115

#define SOURCE_DANNY "DANNY"
#define SOURCE_JACK "JACK"
#define SOURCE_WENDY "WENDY"
#define DATA_ERROR "ERROR"
#define DATA_CONNECT_OK "CONNECTION OK"
#define DATA_OK "DADES OK"
#define DATA_KO "DADES KO"
#define IMAGE_OK "IMATGE OK"
#define IMAGE_KO "IMATGE KO"
#define DATA_FRAME_ERROR "ERROR DE TRAMA"
#define TYPE_CONNECT 'C'
#define TYPE_DATA 'D'
#define TYPE_IMAGE 'I'
#define TYPE_IMAGE_FRAME 'F'
#define TYPE_DISCONNECT 'Q'
#define TYPE_OK_CONNECT 'O'
#define TYPE_OK_DATA 'B'
#define TYPE_OK_IMAGE 'S'
#define TYPE_ERROR_CONNECT 'E'
#define TYPE_ERROR_DATA 'K'
#define TYPE_ERROR_FRAME 'Z'
#define TYPE_ERROR_IMAGE 'R'
#define NUM_HASHTAGS_DATA 5
#define NUM_HASHTAGS_IMAGE 2

//Includes
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//included so that we have the "WeatherFormat" Definition
#include "file_processor.h"

typedef struct {
    char    source[SOURCE_SIZE];
    char    type;
    char    data[DATA_SIZE];
}Frame;

int getFrameData(Frame * frame, char * aux_frame, int read_size);

void readFrame(int sockfd, char * frame);

int weatherDataOK(char data[DATA_SIZE]);

void generateWeatherString(WeatherFormat data, char * formatted_data);

void fillAndSendFrame(int sockfd, char * source, char type, char * data, char * print_message, int print_or_no);

int checkSource(char * source_to_check, char * source_desired);

int checkType(char type_to_check, char type_desired);

void disconnectFrame(int sockfd, char * station_name);

#endif
