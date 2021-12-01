//_danny_file_processor_ Library

#ifndef _file_processor_H
#define _file_processor_H

//for the directory flag O_DIRECTORY
#define _POSIX_C_SOURCE 200809L

//Includes
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

typedef struct {
    char *  weather_station;
    char *  file_directory;
    int     time_to_wait;
    char *  ip_jack;
    int     port_jack;
    char *  ip_wendy;
    int     port_wendy;
}ConfigDanny;

typedef struct {
    char *  ip;
    int     port;
}ConfigGeneral;

typedef struct {
    char * date;
    char * hour;
    char * temperature;
    char * humidity;
    char * atmosphere_pressure;
    char * precipiataion;
    char * file_name;
}WeatherFormat;

typedef struct {
    unsigned char md5sum[33];
    char * file_name;
    char * size_in_bytes;
    unsigned char * image_data;
}ImageFormat;

//Functions and Procedures

//function to load the config from the given file
void loadDannyConfig(int fd, ConfigDanny * config);

void loadGeneralConfig(int fd, ConfigGeneral * config);

WeatherFormat processTextFile(int fd);

WeatherFormat * checkTxtFiles(ConfigDanny config, int * how_many_txt_files);

ImageFormat * checkJPGFiles(ConfigDanny config, int * how_many_jpg_files, int sockfd_jack, int sockfd_wendy);

void printWeatherStationInfo(WeatherFormat data);

int deleteFile(char * full_file_path);

#endif
