#ifndef _GeneratePrint_H
#define _GeneratePrint_H

#define _GNU_SOURCE
#include "../Libraries/file_processor.h"
#include "../Libraries/frame.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

void generateWeatherString(WeatherFormat data, char formatted_data[DATA_SIZE]);

void generateImageString(ImageFormat image, char formatted_data[DATA_SIZE]);

void printConfigInfo(ConfigDanny config);

void printWeatherStationInfo(WeatherFormat data);

void printNumFilesAndNames(ConfigDanny config);

#endif
