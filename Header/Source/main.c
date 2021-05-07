#include <stdlib.h>
#include <stdio.h>        
#include <stdbool.h>      //bool
//#include <unistd.h>
//#include <sys/ioctl.h>     //i2c control
//#include <fcntl.h>         // open
//#include <inttypes.h>      // uint8_t, etc
//#include <linux/i2c-dev.h> // I2C bus definitions
#include <signal.h>        //stop programm with kernel signal
#include <pthread.h>      //create 3 threads for sound sensor
#include <semaphore.h>    //share resource between each thread
//#include <wiringPi.h>     // set gpio pin for servo
//#include <softPwm.h>      // send PWM signal to servo

#include <string.h>
#include <time.h> 

#include "camera.h"
#include "microphone.h"
#include "servo.h"
#include "email.h"

static pid_t pid = 0;
char *i2c_sound[3] = {"/dev/i2c-1","/dev/i2c-3","/dev/i2c-4"};
int servo_position(int position);

int main()
{
  int iret[3]; //thread handler
  int servo_pin = 25;
  pthread_t sound_thread[3]; //create 3 threads for microphone
  time_t now;
  char directory[1000] = "/home/pi/Desktop/HelloWorld/videoFolder/"; //set directory

  sem_init(&mutex,0,1);
  wiringPiSetupGpio();
  pinMode(servo_pin,PWM_OUTPUT);
  digitalWrite(servo_pin, LOW);
   
 
while(1){
  //create 3 threads for sound sampling
  for(int i = 0; i<3; i++){
    if(iret[i]=pthread_create(&sound_thread[i], NULL, &sound_funct, i2c_sound[i])){
      printf("Thread creation failed: %d\n", iret[i]);
    }
  }

  //wait till thread complete
  //three sound sensor compete to find the event
  //if a thread found the event first
  //stop other threads and return with a location of the event
  for(int i= 0; i<3;i++){
    pthread_join(sound_thread[i], NULL);
  }

  //check for which sensor detect it first and move the servo to the position
  if(pos==1){
    printf("Move servo to position %u\n", pos);
    sendEmail("  ""Left side breaking detected "" ");
  }else if(pos==2){
    printf("Move servo to position %u\n", pos);
    sendEmail("  ""Center breaking detected on the left"" ");
  }else if(pos == 3){
    printf("Move servo to position %u\n", pos);
    sendEmail("  ""Right ride breaking detected on the left"" ");
  }

  //move servo to the found position
  softPwmCreate(servo_pin,servo_position(pos),100); //create pulse only if needed
  taken = false;                                    //restart the found status
  delay(500);                                       //wait for servo to move  
  softPwmStop(servo_pin);                           // stop servo when not needed



  // //record video for 5s
  // time(&now);                    //get current time as name of file 
  // strcat(directory,ctime(&now)); //and combine with the directory
  // strcat(directory,".h264");
  // printf("Recording video for 5 secs...");
  // startVideo(directory, "-w 640 -h 480");//-cfx -rot 180"); 
  // fflush(stdout);
  // sleep(5);//change recording time here
  // stopVideo();
  // printf("\nVideo stopped - exiting in 2 secs.\n");
  // sleep(2);
}
  
  sem_destroy(&mutex); //destroy semaphore

  return 0;
} //end main


////////////////////////////////////////////////////////////////////////////




//move servo to a specific location
/*int servo_position(int position){ //0 25 90 135 or 180 degree
  int pulse;
  switch (position)
  {
    case 0:
      pulse = 5;
      break;
    case 1:
      pulse = 10;
      break;
    case 2:
      pulse = 15;
      break;
    case 3:
      pulse = 20;
      break;
    case 4:
      pulse = 25;
      break;
    default:
      pulse = 5;
      break;
    return pulse;
  }
}

void startVideo(char *filename, char *options) {
    if ((pid = fork()) == 0) {
        char **cmd;

        // count tokens in options string
        int count = 0;
        char *copy;

        copy = strdup(options);
        if (strtok(copy, " \t") != NULL) {
            count = 1;
            while (strtok(NULL, " \t") != NULL)
                count++;
        }

        cmd = malloc((count + 8) * sizeof(char **));
        free(copy);

        // if any tokens in options, 
        // copy them to cmd starting at positon[1]
        if (count) {
            int i;
            copy = strdup(options);
            cmd[1] = strtok(copy, " \t");
            for (i = 2; i <= count; i++)
                cmd[i] = strtok(NULL, " \t");
        }

        // add default options
        cmd[0] = "raspivid"; // executable name
        cmd[count + 1] = "-n"; // no preview
        cmd[count + 2] = "-t"; // default time (overridden by -s)
                               // but needed for clean exit
        cmd[count + 3] = "10"; // 10 millisecond (minimum) time for -t
        cmd[count + 4] = "-s"; // enable USR1 signal to stop recording
        cmd[count + 5] = "-o"; // output file specifer
        cmd[count + 6] = filename;
        cmd[count + 7] = (char *)0; // terminator
        execv("/usr/bin/raspivid", cmd);
    }
}

void stopVideo(void) {
    if (pid) {
        kill(pid, 10); // seems to stop with two signals separated
                       // by 1 second if started with -t 10 parameter
        sleep(1);
        kill(pid, 10);
    }
}*/




