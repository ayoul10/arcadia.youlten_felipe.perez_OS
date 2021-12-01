#include "Lloyd.h"

void calculateAverageLloyd(LloydData * lloyd_data, int found_pos, int old_num_times, int new_num_times, SharedMemData * sh_data ){
  lloyd_data[found_pos].avg_temperature = ((lloyd_data[found_pos].avg_temperature * old_num_times) + sh_data->temperature) / new_num_times;
  lloyd_data[found_pos].avg_humidity = ((lloyd_data[found_pos].avg_humidity * old_num_times) + sh_data->humidity) / new_num_times;
  lloyd_data[found_pos].avg_atmosphere_pressure = ((lloyd_data[found_pos].avg_atmosphere_pressure * old_num_times) + sh_data->atmosphere_pressure) / new_num_times;
  lloyd_data[found_pos].avg_precipiataion = ((lloyd_data[found_pos].avg_precipiataion * old_num_times) + sh_data->precipiataion) / new_num_times;
}

void updateLloydValues(LloydData * lloyd_data, int num_stations, SharedMemData * sh_data){

  lloyd_data[num_stations-1].avg_temperature = sh_data->temperature;
  lloyd_data[num_stations-1].avg_humidity = sh_data->humidity;
  lloyd_data[num_stations-1].avg_atmosphere_pressure = sh_data->atmosphere_pressure;
  lloyd_data[num_stations-1].avg_precipiataion = sh_data->precipiataion;
  lloyd_data[num_stations-1].num_times = 1;

}

void stringWriterLloyd(char * station_name, LloydData lloyd_data_single, int fd){
  char string[256];

  sprintf(string, "Station name: %s",station_name); //spritnf -- write formatted data to string
  write(fd, string, strlen(string));

  sprintf(string, "\nStation average temperature: %.3f", lloyd_data_single.avg_temperature); //spritnf -- write formatted data to string
  write(fd, string, strlen(string));

  sprintf(string, "\nStation average humidity: %d", lloyd_data_single.avg_humidity); //spritnf -- write formatted data to string
  write(fd, string, strlen(string));

  sprintf(string, "\nStation average precipiataion: %.3f", lloyd_data_single.avg_precipiataion); //spritnf -- write formatted data to string
  write(fd, string, strlen(string));

  sprintf(string, "\nStation average atmospheric pressure: %.3f\n\n", lloyd_data_single.avg_atmosphere_pressure); //spritnf -- write formatted data to string
  write(fd, string, strlen(string));
}

void LloydWriteToFile(LloydData * lloyd_data, int stations, char * stations_array[MAX_CONNECTIONS]){

  char string[256];
  int fd = -1;

  if(stations > 0){
    fd = open("Hallorann.txt", O_WRONLY | O_CREAT, 0600);

    if(fd == -1){
      sprintf(string, "Unable to open Hallorann file!\n");
      write(STDOUT_FILENO, string, strlen(string));
    }else {
      for (int i = 0; i < stations; i++) {
        stringWriterLloyd(stations_array[i], lloyd_data[i], fd);
      }
    }
  }else{
    fd = open("Hallorann.txt", O_WRONLY | O_CREAT | O_TRUNC, 0600);
    //clear Hallorann file before writing

    if(fd == -1){
      sprintf(string, "Unable to open Hallorann file!\n");
      write(STDOUT_FILENO, string, strlen(string));
    }else {
      sprintf(string, "No stations yet!"); //sprintf -- write formatted data to string
      write(fd, string, strlen(string));
    }
  }
  close(fd);
}

void lloydProcess(SharedMemData * sh_data, semaphore sem_sync_lloyd, semaphore sem_lloyd_done){
  LloydData * lloyd_data = NULL;
  int num_stations=0;
  char * array_stations[MAX_CONNECTIONS];
  char string[256];

  sigset_t block_list;
  //emtpy the set to avoid a valgrind error
  sigemptyset(&block_list);
  sigaddset(&block_list, SIGALRM);

  while(1){

    SEM_wait(&sem_sync_lloyd);

    if(strlen(sh_data->name) <= 0) {
      break;
    }else if(strcmp(sh_data->name, LLOYD_WRITE)){

      // BLOCK SIGALRM
      sigprocmask(SIG_BLOCK, &block_list, NULL);

      sprintf(string, "\nstation name in lloyd: %s\n", sh_data->name); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));

      // if no stations there (first station to get sent to lloyd)
      if(num_stations == 0){

        sprintf(string, "Didn't have stations in Lloyd\n");
        write(STDOUT_FILENO, string, strlen(string));

        num_stations++;

        array_stations[num_stations-1] = (char *) malloc(sizeof(char) * strlen(sh_data->name) + 1);
        strcpy(array_stations[num_stations-1], sh_data->name);
        lloyd_data = (LloydData *) realloc(lloyd_data, num_stations * sizeof(LloydData));

        updateLloydValues(lloyd_data, num_stations, sh_data);

      // already stations there
      }else{
        int found_pos = -1;

        for (int i = 0; i < num_stations; i++) {
          if(!strcmp(sh_data->name, array_stations[i])){
            found_pos=i;
            break;
          }
        }

        // if station not found
        if(found_pos == -1){

          sprintf(string, "Had stations, not found\n");
          write(STDOUT_FILENO, string, strlen(string));

          num_stations++;

          array_stations[num_stations-1] = (char *) malloc(sizeof(char) * strlen(sh_data->name) + 1);
          strcpy(array_stations[num_stations-1], sh_data->name);

          lloyd_data = (LloydData *) realloc(lloyd_data, num_stations * sizeof(LloydData));

          updateLloydValues(lloyd_data, num_stations, sh_data);

        //if station found
        }else{

          sprintf(string, "Had stations, found\n");
          write(STDOUT_FILENO, string, strlen(string));

          int old_num_times = lloyd_data[found_pos].num_times;
          int new_num_times = ++lloyd_data[found_pos].num_times;

          calculateAverageLloyd(lloyd_data, found_pos, old_num_times, new_num_times, sh_data);

        }
      }
      //signal that lloyd has finished reading from the shared memory
      SEM_signal(&sem_lloyd_done);

    // UNBLOCK SIGALRM
    sigprocmask(SIG_UNBLOCK, &block_list, NULL);
    }else if(!strcmp(sh_data->name, LLOYD_WRITE)){
      sprintf(string, "\nWritting to file...\n"); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));

      LloydWriteToFile(lloyd_data, num_stations, array_stations);

      sprintf(string, "\nWrite finished...\n"); //spritnf -- write formatted data to string
      write(STDOUT_FILENO, string, strlen(string));
    }
  }

  //free memory
  if (num_stations > 0){
    for (int i = 0; i < num_stations; i++) {
      free(array_stations[i]);
    }
    free(lloyd_data);
  }

  raise(SIGTERM);
}
