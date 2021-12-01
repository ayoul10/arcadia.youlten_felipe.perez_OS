#include "file_processor.h"

/*
  Function to read any type of string from a file with a specified separator
  by using the file descriptor
*/

void readStringFromFile(int fd, char ** str, char separator, int last){

  char buffer[100];
  int index=0;

  read(fd, &buffer[index], sizeof(char));

  // remove the / at the beggining of the word for the directory
  if(buffer[index] == '/'){
    index =0;
    read(fd, &buffer[index], sizeof(char));
  }

  if (!last) {
    while (buffer[index] != separator) {
        index++;
        read(fd, &buffer[index], sizeof(char));
    }
  }else{
    index++;
    while (read(fd, &buffer[index], sizeof(char)) && buffer[index] != separator) {
        index++;
    }
  }

  buffer[index-1]= '\0';

  *str = (char *) malloc((index * sizeof(char))); //malloc length of string

  //copy the name to the final variable
  strcpy(*str, buffer);
}

/*
  Function to read any integer from a file with a specified separator
  by using the file descriptor
*/

void readNumberFromFile(int fd, int * num, char separator){
  char buffer[100];
  //char string[200];
  int index=0;

  //read the amount of time to wait between files
  index=0;
  read(fd, &buffer[index], sizeof(char));
  while (buffer[index] != separator) {
      index++;
      read(fd, &buffer[index], sizeof(char));
  }
  buffer[index]= '\0';

  *num = atoi(buffer);
}

/*
  Get the information for the config file for Danny and put it in a struct that holds the given information
  the config variable is passed by reference.
  Heavliy uses readNumberFromFile and readStringFromFile
*/

void loadDannyConfig(int fd, ConfigDanny * config){

  //read the name of the weaher station
  readStringFromFile(fd, &(config->weather_station), '\n', 0);

  //read the directory
  readStringFromFile(fd, &(config->file_directory), '\n', 0);

  //Read the amount of time to wait between file checks.
  readNumberFromFile(fd, &(config->time_to_wait), '\n');

  //Read Jack's IP
  readStringFromFile(fd, &(config->ip_jack), '\n', 0);

  //Read Jack's port
  readNumberFromFile(fd, &(config->port_jack), '\n');

  //Read Wendy's IP
  readStringFromFile(fd, &(config->ip_wendy), '\n', 0);

  //Read Wendy's port
  readNumberFromFile(fd, &(config->port_wendy), '\n');

}

void loadGeneralConfig(int fd, ConfigGeneral * config){

  //Read Jack's IP
  readStringFromFile(fd, &(config->ip), '\n', 0);

  //Read Jack's port
  readNumberFromFile(fd, &(config->port), '\n');
}

/*
  Get the information for the text files in the danny directory put it in a struct that holds the given information
  heavliy uses  readStringFromFile
*/
WeatherFormat processTextFile(int fd){
  WeatherFormat data;
  //read date
  readStringFromFile(fd, &(data.date), '\n', 0);
  //read hour
  readStringFromFile(fd, &(data.hour), '\n', 0);
  //read temp
  readStringFromFile(fd, &(data.temperature), '\n', 0);
  //read humidity
  readStringFromFile(fd, &(data.humidity), '\n', 0);
  //read pressure
  readStringFromFile(fd, &(data.atmosphere_pressure), '\n', 0);
  //read percipiataion
  readStringFromFile(fd, &(data.precipiataion), '\n', 1);

  return data;
}



/*
  Determine if a file is a .txt or not. we do this by passing the function the string of the file d_name
  we read until the ., and then check if the information after it is txt or not
*/
int isTxt(char * filename){
  char string[10];
  int flag=0;
  int j=0;

  for (int i = 0; i < (int) strlen(filename); i++) {
    if(flag ==1){
      string[j]= filename[i];
      j++;
    }
    if(filename[i] == '.'){
      flag =1;
    }
  }
  string[j] = '\0';

  return !strcmp("txt", string);
}

int isJPG(char * filename){
  char string[10];
  int flag=0;
  int j=0;

  for (int i = 0; i < (int) strlen(filename); i++) {
    if(flag ==1){
      string[j]= filename[i];
      j++;
    }
    if(filename[i] == '.'){
      flag =1;
    }
  }
  string[j] = '\0';

  return !strcmp("jpg", string);
}

/*
Delete a file from the directory that we want using the remove() function from stdio.h
*/
int deleteFile(char * full_file_path){
  int r=0;
  return r = remove(full_file_path);
}

void getFullFilePathAndName(char full_file_path[200], char file_path[200], struct dirent *dp, ConfigDanny config){
  size_t i=0;
  size_t j=0;
  //get the full file path to open it
  for (i = 0; i < strlen(config.file_directory); i++) {
      full_file_path[i] = config.file_directory[i];
  }

  full_file_path[i] = '/';
  i++;
  for (j = 0; j < strlen(dp->d_name); j++) {
    full_file_path[i] = dp->d_name[j];
    file_path[j] = dp->d_name[j];
    i++;
  }

  full_file_path[i] = '\0';
  file_path[j] = '\0';
}

/*

*/
WeatherFormat * checkTxtFiles(ConfigDanny config, int * how_many_txt_files){
  struct dirent *dp;
  int fd_directory;
  int fd_file;
  DIR *d;
  int num_txt_files=0;
  char full_file_path[200];
  char file_path[200];
  char string[300];

  //WeatherFormat data;
  WeatherFormat * data = calloc(0,0);

  if ((d = fdopendir((fd_directory = open(config.file_directory, O_RDONLY)))) == NULL) {
    sprintf(string, "Unable to open directory!\n");
    write(STDOUT_FILENO, string, strlen(string));
    raise(SIGINT);
  }else{
    while ((dp = readdir(d)) != NULL) {
      //skip hidden files
      if (dp->d_name[0] == '.')
        continue;

      if (isTxt(dp->d_name)) {
        num_txt_files++;

        data = (WeatherFormat *) realloc(data, sizeof(WeatherFormat) * num_txt_files);

        getFullFilePathAndName(full_file_path, file_path, dp, config);

        //open the file we found to be a txt
        fd_file = open(full_file_path, O_RDONLY, 0600);
        if(fd_file < 0 ){
            sprintf(string, "Unable to open weather file!\n");
            write(STDOUT_FILENO, string, strlen(string));
        }else{

          //process the file and print the data found
          data[num_txt_files-1] = processTextFile(fd_file);

          data[num_txt_files-1].file_name = (char *) malloc(sizeof(char) * (strlen(file_path) +1));
          strcpy(data[num_txt_files-1].file_name, file_path);

          //close the file
          close(fd_file);
        }

        //delete the file
        if(deleteFile(full_file_path) != 0){
          sprintf(string, "Problem deleting file: %s\n", dp->d_name);
          write(STDOUT_FILENO, string, strlen(string));
        }
      }
    }
    closedir(d); // note this implicitly closes fd_directory
    *how_many_txt_files = num_txt_files;
  }
  // return array
  return data;
  // free malloc'd memory OUTSIDE!!!
}

ImageFormat * checkJPGFiles(ConfigDanny config, int * how_many_jpg_files, int sockfd_jack, int sockfd_wendy){
  struct dirent *dp;
  int fd_directory;
  int fd_file;
  unsigned long long index =0;
  char buffer;
  DIR *d;
  int num_jpg_files=0;
  int pipeAB[2];
  int pid = -1;
  char md5_buffer[100];
  char string[300];
  char full_file_path[200];
  char file_path[200];
  char * path = "/usr/bin/md5sum";  // md5sum is located in /usr/bin/
  int read_size=0;

  ImageFormat * images = calloc(0,0);

  if ((d = fdopendir((fd_directory = open(config.file_directory, O_RDONLY)))) == NULL) {
    sprintf(string, "Unable to open directory!\n");
    write(STDOUT_FILENO, string, strlen(string));
      exit(1);
  }else{

    while ((dp = readdir(d)) != NULL) {
      //skip hidden files
      if (dp->d_name[0] == '.')
        continue;

      if (isJPG(dp->d_name)) {
        num_jpg_files++;

        images = (ImageFormat *) realloc(images, sizeof(ImageFormat) * num_jpg_files);

        getFullFilePathAndName(full_file_path, file_path, dp, config);

        // Set up pipe for md5sum
        if(pipe(pipeAB) == -1){
      		sprintf(string, "Error when creating the first pipe\n");
      		write(1, string, strlen(string));

      	}else{
          fd_file = open(full_file_path, O_RDONLY, 0600);
          if(fd_file == -1){
            sprintf(string, "Unable to open jpg file!\n");
            write(STDOUT_FILENO, string, strlen(string));

          }else{
            pid = fork();
          	if (pid == 0){  // child
              // close fds from parent
              close(fd_file);
              closedir(d); // note this implicitly closes fd_directory
              close(sockfd_jack);
              close(sockfd_wendy);

              //close the read pipe
              close(pipeAB[0]);
              //get md5sum of the file
              dup2(pipeAB[1], 1);
              close(pipeAB[1]);
              execl(path, path, full_file_path, NULL);
            }

            close(pipeAB[1]);  // close the write end of the pipe in the parent

            while (read(pipeAB[0], md5_buffer, sizeof(md5_buffer)) != 0){}
            wait(NULL);

            close(pipeAB[0]); // close the read end of the pipe in the parent

            //process output of md5sum
            for (int i = 0; i < 32; i++) {
              images[num_jpg_files-1].md5sum[i] = md5_buffer[i];
            }
            images[num_jpg_files-1].md5sum[32] = '\0';

            // Process file to get binary data
            index =0;
            images[num_jpg_files-1].image_data = calloc(0,0);
            read_size = read(fd_file, &buffer, sizeof(unsigned char));
            while (read_size != 0) {
              //increase num positions in the string
              index++;
              images[num_jpg_files-1].image_data = (unsigned char *) realloc(images[num_jpg_files-1].image_data, index);

              images[num_jpg_files-1].image_data[index-1] = buffer;
              read_size = read(fd_file, &buffer, sizeof(unsigned char));
            }

            // set the file name and the size in bytes of the file
            sprintf(string, "%llu", index);
            images[num_jpg_files-1].size_in_bytes = (char *) malloc(sizeof(char) * (strlen(string) +1));
            strcpy(images[num_jpg_files-1].size_in_bytes, string);

            images[num_jpg_files-1].file_name = (char *) malloc(sizeof(char) * (strlen(file_path) +1));
            strcpy(images[num_jpg_files-1].file_name, file_path);

            // close the file
            close(fd_file);
          }
        }

        //delete the file
        if(deleteFile(full_file_path) == 1){
          sprintf(string, "Problem deleting file: %s\n", dp->d_name);
          write(STDOUT_FILENO, string, strlen(string));
        }
      }
    }
    closedir(d); // note this implicitly closes fd_directory
    *how_many_jpg_files = num_jpg_files;
  }

  return images;
  //free malloc'd memory OUTSIDE!!!
}
