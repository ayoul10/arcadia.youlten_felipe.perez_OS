#include "WendyThreadManager.h"
void freeImageInfo(Image image){
  free(image.image_data);
  free(image.file_name);
  free(image.file_size);

}
Image getImageData(Frame frame){

  int index = 0;
  int num_tags = 0;

  Image image;
  image.file_name = calloc(0,0);
  image.file_size = calloc(0,0);

  // parse all of the image data sent
  for (int i = 0; i < (int) strlen(frame.data); i++) {

    if(frame.data[i] == '#'){
      num_tags++;
      index=0;

    }else{
      switch (num_tags){
        //file name
        case 0:
          image.file_name = (char *) realloc(image.file_name, index+2);
          image.file_name[index] = frame.data[i];
          index++;
          image.file_name[index] = '\0';
        break;

        //file size
        case 1:
          image.file_size = (char *) realloc(image.file_size, index+2);
          image.file_size[index] = frame.data[i];
          index++;
          image.file_size[index] = '\0';
        break;

        //m5sum
        case 2:
          image.md5sum[index] = frame.data[i];
          index++;
          image.md5sum[index] = '\0';
        break;
      }
    }
  }

  return image;
}

void DannyProcessWendy (ThreadData *aux){
  struct pollfd poll_fd;
  Frame frame;
  char received_frame[115];
  int read_size;
  char string[200];
  char data_buffer[DATA_SIZE];
  int ret = 0;

  int sockfd = aux->sockfd;
  int unbound_fd = aux->unbound_fd;

  unsigned long long bytes_received =0;
  unsigned long long bytes_remaining =0;
  char md5_buffer[100];
  char * path = "/usr/bin/md5sum";  // md5sum is located in /usr/bin/
  int fd_file = -1;
  int pipeBA[2];
  int pid = -1;
  int equal = 1;

  char * station_name = (char *) malloc(strlen(aux->frame.data) * sizeof(char) + 1);
  strcpy(station_name, aux->frame.data);

  sprintf(string, "New Connection: %s\n", station_name);
  write(STDOUT_FILENO, string, strlen(string));

  // check what type of information we're recieving from danny
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
      if(getFrameData(&frame, received_frame, read_size) || strstr(frame.data, ".jpg") == NULL){
        // send back an error frame frame and print Frame Error
        // strcpy(data_buffer, DATA_FRAME_ERROR);
        // fillAndSendFrame(sockfd, SOURCE_WENDY, TYPE_ERROR_FRAME, data_buffer, "Frame Error", 1);

        //sprintf(string, "Read size frame error: %d Image Data\n", read_size);
        //write(STDOUT_FILENO, string, strlen(string));

      }else if(checkType(frame.type, TYPE_DISCONNECT)) {
        // disconnect and free the station name memory allocation
        disconnectFrame(sockfd, station_name);
        sprintf(string, "Disconnected from Wendy\n");
        write(STDOUT_FILENO, string, strlen(string));
        free(station_name);
        return;

      }else{
        // allocate enough memory for the incoming image data
        // convert information from frame to Image type
        Image image = getImageData(frame);

        //set the bytes remaining as the file size. we can always subtract from thsi value
        bytes_remaining = strtoull(image.file_size, NULL, 10);
        //malloc the exact amount of space needed
        image.image_data = (unsigned char *) malloc(sizeof(unsigned char) * bytes_remaining);

        bytes_received = 0;

        while(1){

          read_size = read(sockfd, received_frame, FRAME_SIZE);

          getFrameData(&frame, received_frame, read_size);

          if(bytes_remaining > DATA_SIZE){
            for (unsigned long long j = 0; j < DATA_SIZE; j++) {
              image.image_data[j+bytes_received] = (unsigned char) frame.data[j];
            }

            bytes_received = bytes_received + DATA_SIZE;
            bytes_remaining = bytes_remaining - DATA_SIZE;
          }else{
            //send all remaining info
            for (unsigned long long j = 0; j < bytes_remaining; j++) {
              image.image_data[j+bytes_received] = (unsigned char) frame.data[j];
            }

            bytes_received = bytes_received + bytes_remaining;

            break;
          }
        }

        sprintf(string, "All info recieved from danny\n");
        write(STDOUT_FILENO, string, strlen(string));

        // Write to jpg file
        fd_file = open(image.file_name, O_WRONLY | O_TRUNC | O_CREAT, 0600);

        if(fd_file == -1){
          sprintf(string, "Unable to open jpg file!\n");
          write(STDOUT_FILENO, string, strlen(string));
        }else{
          for (unsigned long long i = 0; i < bytes_received; i++) {
            write(fd_file, &image.image_data[i], 1);
          }
          close(fd_file);
        }

        // Do md5sum of written file
        if(pipe(pipeBA) == -1){
          sprintf(string, "Error when creating the first pipe\n");
          write(1, string, strlen(string));

        }else{

          equal = 1;
          pid = fork();
          if (pid == 0){  // child
            // close fds from parent
            close(sockfd);
            close(unbound_fd);

            //close the read pipe
            close(pipeBA[0]);
            //get md5sum of the file
            dup2(pipeBA[1], 1);
            close(pipeBA[1]);
            execl(path, path, image.file_name, NULL);
          }

          close(pipeBA[1]);  // close the write end of the pipe in the parent

          while (read(pipeBA[0], md5_buffer, sizeof(md5_buffer)) != 0){}
          wait(NULL);
          close(pipeBA[0]);

          sprintf(string, "Child finished\n");
          write(STDOUT_FILENO, string, strlen(string));

          //compare output of md5sum
          for (int i = 0; i < 32; i++) {
            if(image.md5sum[i] != md5_buffer[i])
              equal = 0;
          }

          // Send back frame notifying Danny wether the md5sums were the same or not
          bzero(data_buffer, DATA_SIZE);
          if (equal) {
            strcpy(data_buffer, IMAGE_OK);

            char temp[200];

            strcpy(temp, BARRY_DIR);
            strcat(temp, image.file_name);

            rename(image.file_name, temp);

            fillAndSendFrame(sockfd, SOURCE_WENDY, TYPE_OK_IMAGE, data_buffer, "MD5SUM OK", 1);
          }else{

            if(deleteFile(image.file_name) != 0){
              sprintf(string, "Problem deleting file: %s\n", image.file_name);
              write(STDOUT_FILENO, string, strlen(string));
            }

            strcpy(data_buffer, IMAGE_KO);
            fillAndSendFrame(sockfd, SOURCE_WENDY, TYPE_ERROR_IMAGE, data_buffer, "MD5SUM KO", 1);
          }
        }
        freeImageInfo(image);
      }
    }
  }
}
