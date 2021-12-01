#include "GeneratePrint.h"

void generateWeatherString(WeatherFormat data, char formatted_data[DATA_SIZE]){
  bzero(formatted_data, DATA_SIZE);
  strcpy(formatted_data, data.date);
  strcat(formatted_data, "#");
  strcat(formatted_data, data.hour);
  strcat(formatted_data, "#");
  strcat(formatted_data, data.temperature);
  strcat(formatted_data, "#");
  strcat(formatted_data, data.humidity);
  strcat(formatted_data, "#");
  strcat(formatted_data, data.atmosphere_pressure);
  strcat(formatted_data, "#");
  strcat(formatted_data, data.precipiataion);
}

void generateImageString(ImageFormat image, char formatted_data[DATA_SIZE]){
  bzero(formatted_data, DATA_SIZE);
  strcpy(formatted_data, image.file_name);
  strcat(formatted_data, "#");
  strcat(formatted_data, image.size_in_bytes);
  strcat(formatted_data, "#");
  strcat(formatted_data, (char *) image.md5sum);
}

void printNumFilesAndNames(ConfigDanny config){

  struct dirent *dp;
  int fd_directory;
  DIR *d;
  int num_files=0;
  char string[300];

  if ((d = fdopendir((fd_directory = open(config.file_directory, O_RDONLY)))) == NULL) {
    sprintf(string, "Unable to open directory!\n");
    write(STDOUT_FILENO, string, strlen(string));
    raise(SIGINT);
  }else{
    while ((dp = readdir(d)) != NULL) {
      //skip hidden files
      if (dp->d_name[0] == '.')
        continue;
      num_files++;
    }
  }
  sprintf(string, "Num files: %d\n", num_files);
  write(STDOUT_FILENO, string, strlen(string));

  rewinddir(d);

  while ((dp = readdir(d)) != NULL) {
    //skip hidden files
    if (dp->d_name[0] == '.')
      continue;
    sprintf(string, "%s\n", dp->d_name);
    write(STDOUT_FILENO, string, strlen(string));
  }

  closedir(d);

}

void printWeatherStationInfo(WeatherFormat data){

  char string[255];
  sprintf(string, "\n%s\n", data.file_name);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n", data.date);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n", data.hour);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n", data.temperature);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n", data.humidity);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n", data.atmosphere_pressure);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "%s\n\n", data.precipiataion);
  write(STDOUT_FILENO, string, strlen(string));

}

void printConfigInfo(ConfigDanny config){
  char string[255];
  sprintf(string, "Weather station: %s\n", config.weather_station);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "Weather station: %s\n", config.file_directory);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "Wait time: %d\n", config.time_to_wait);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "IP Jack: %s\n", config.ip_jack);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "Port Jack: %d\n", config.port_jack);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "IP Wendy: %s\n", config.ip_wendy);
  write(STDOUT_FILENO, string, strlen(string));

  sprintf(string, "Port Wendy: %d\n", config.port_wendy);
  write(STDOUT_FILENO, string, strlen(string));
}
