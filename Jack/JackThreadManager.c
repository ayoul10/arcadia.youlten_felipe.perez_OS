#include "JackThreadManager.h"

void processCorrectData(pthread_mutex_t sem_mutex_shmem, semaphore sem_sync_lloyd, semaphore sem_lloyd_done, Frame frame, char * station_name, SharedMemData * sh_data){
  pthread_mutex_lock(&sem_mutex_shmem);
  SharedMemData data = processDannyWeatherinfo(frame, station_name);

  //PLACE DATA IN SHMEM
  strcpy(sh_data->name, data.name);
  sh_data->temperature = data.temperature;
  sh_data->humidity = data.humidity;
  sh_data->atmosphere_pressure = data.atmosphere_pressure;
  sh_data->precipiataion = data.precipiataion;

  //SIGNAL SEMAPHORE FOR LLOYD TO READ THE SHMEM DATA
  SEM_signal(&sem_sync_lloyd);

  //WAIT FOR LLOYD TO FINISH READING AND PROCESSING THE DATA
  SEM_wait(&sem_lloyd_done);

  // SIGNAL SEMAPHORE THAT SHARED MEMORY IS AVAILABLE TO BE WRITTEN TO
  pthread_mutex_unlock(&sem_mutex_shmem);
}

void convertCharToValue(SharedMemData * aux_shmem, char * temperature, char * humidity, char * atmosphere_pressure, char * precipiataion, char * station_name){
  aux_shmem->temperature = atof(temperature);
  aux_shmem->humidity = atoi(humidity);
  aux_shmem->atmosphere_pressure = atof(atmosphere_pressure);
  aux_shmem->precipiataion = atof(precipiataion);
  strcpy(aux_shmem->name, station_name);
}

void freeWeatherDataPointers(char * temperature, char * humidity, char * atmosphere_pressure, char * precipiataion){
    free(temperature);
    free(humidity);
    free(atmosphere_pressure);
    free(precipiataion);
}


void DannyProcessJack (ThreadData *aux, SharedMemData * sh_data, semaphore sem_sync_lloyd, semaphore sem_lloyd_done, pthread_mutex_t sem_mutex_shmem){

  int ret =0;
  struct pollfd poll_fd;
  Frame frame;
  char received_frame[115];
  int read_size;
  char string[200];
  char data_buffer[DATA_SIZE];

  int sockfd = aux->sockfd;
  //int danny_num = aux->danny_num;
  char * station_name = (char *) malloc(strlen(aux->frame.data) * sizeof(char) + 1);
  strcpy(station_name, aux->frame.data);

  sprintf(string, "New Connection: %s\n", station_name);
  write(STDOUT_FILENO, string, strlen(string));

  // check what type of information we're recieving from jack
  while(1){
    poll_fd.fd = sockfd;
    poll_fd.events = POLLNVAL | POLLRDHUP | POLLIN;

    ret = poll(&poll_fd, 1, 0);

    while(!ret){
      ret = poll(&poll_fd, 1, 0);
    }

    if(poll_fd.revents & (POLLNVAL | POLLRDHUP)){
      sprintf(string, "Closing inside thread...\n");
      write(STDOUT_FILENO, string, strlen(string));
      free(station_name);
      return;
    }else if(poll_fd.revents & POLLIN){
      // read data from danny
      read_size = read(sockfd, received_frame, FRAME_SIZE);
      sprintf(string, "Recieving data...\n");
      write(STDOUT_FILENO, string, strlen(string));

      bzero(data_buffer, DATA_SIZE);
      if(getFrameData(&frame, received_frame, read_size)){
        // send back an error frame frame and print Frame Error
        strcpy(data_buffer, DATA_FRAME_ERROR);
        fillAndSendFrame(sockfd, SOURCE_JACK, TYPE_ERROR_FRAME, data_buffer, "Frame Error", 1);

      }else if(((strcmp(frame.source, SOURCE_DANNY) != 0) && (frame.type != TYPE_DATA || frame.type != TYPE_DISCONNECT))){
        // send back an error data frame and print Data Error
        strcpy(data_buffer, DATA_KO);
        fillAndSendFrame(sockfd, SOURCE_JACK, TYPE_ERROR_DATA, data_buffer, "Data Error", 1);

      }else if(frame.type == 'Q'){
        // disconnect and free the station name memory allocation
        disconnectFrame(sockfd, station_name);
        return;

      }else{
        strcpy(data_buffer, DATA_OK);
        fillAndSendFrame(sockfd, SOURCE_JACK, TYPE_OK_DATA, data_buffer, "Data OK", 1);

        if(frame.type == TYPE_DATA){
          // process the data received
          processCorrectData(sem_mutex_shmem, sem_sync_lloyd, sem_lloyd_done, frame, station_name, sh_data);
        }
      }
    }
  }
}

//prepare data to be written to shmem (without station name)
SharedMemData processDannyWeatherinfo(Frame frame, char * station_name){
  int num_tags =0;
  int index=0;
  char * temperature;
  char * humidity;
  char * atmosphere_pressure;
  char * precipiataion;
  SharedMemData aux_shmem;

  temperature = calloc(0,0);
  humidity = calloc(0,0);
  atmosphere_pressure = calloc(0,0);
  precipiataion = calloc(0,0);

  //parse all of the atmospheric data to be passed to the shared memory
  for (int i = 0; i < (int) strlen(frame.data); i++) {

    if(frame.data[i] == '#'){
      num_tags++;
      index=0;

    }else{
      switch (num_tags){

        //Temperature
        case 2:
        temperature = (char *) realloc(temperature, index+2);
        temperature[index] = frame.data[i];
        index++;
        temperature[index] = '\0';
          break;

        //Humidity
        case 3:
        humidity = (char *) realloc(humidity, index+2);
        humidity[index] = frame.data[i];
        index++;
        humidity[index] = '\0';
          break;

        //pressure
        case 4:
        atmosphere_pressure = (char *) realloc(atmosphere_pressure, index+2);
        atmosphere_pressure[index] = frame.data[i];
        index++;
        atmosphere_pressure[index] = '\0';
          break;

        //precipiataion
        case 5:
        precipiataion = (char *) realloc(precipiataion, index+2);
        precipiataion[index] = frame.data[i];
        index++;
        precipiataion[index] = '\0';
          break;
      }
    }
  }

  convertCharToValue(&aux_shmem, temperature, humidity, atmosphere_pressure, precipiataion, station_name);
  freeWeatherDataPointers(temperature, humidity, atmosphere_pressure, precipiataion);

  return aux_shmem;
}
