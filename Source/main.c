#include <stdlib.h>
#include <stdio.h>        
#include <stdbool.h>      //bool
#include <signal.h>        //stop programm with kernel signal
#include <pthread.h>      //create 3 threads for sound sensor
#include <semaphore.h>    //share resource between each thread
//#include <wiringPi.h>     // set gpio pin for servo
//#include <softPwm.h>      // send PWM signal to servo

#include <string.h>
#include <time.h>   //get current time

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
  char directory[1000] = "/home/pi/Desktop/HelloWorld/videoFolder/"; //set directory where video is saved

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
    sendEmail("""Left side breaking detected """);
  }else if(pos==2){
    printf("Move servo to position %u\n", pos);
    sendEmail("""Center breaking detected on the left""");
  }else if(pos == 3){
    printf("Move servo to position %u\n", pos);
    sendEmail("""Right ride breaking detected on the left""");
  }

  //move servo to the found position
  softPwmCreate(servo_pin,servo_position(pos),100); //create pulse only if needed
  taken = false;                                    //restart the found status
  delay(500);                                       //wait for servo to move  
  softPwmStop(servo_pin);                           // stop servo when not needed

  //record video for 5s
  time(&now);                    //get current time as name of file 
  strcat(directory,ctime(&now)); //and combine with the directory
  strcat(directory,".h264");
  printf("Recording video for 5 secs...");
  startVideo(directory, "-w 640 -h 480");//-cfx -rot 180"); 
  fflush(stdout);
  sleep(5);//change recording time here
  stopVideo();
  printf("\nVideo stopped - exiting in 2 secs.\n");
  sleep(2);
}
  
  sem_destroy(&mutex); //destroy semaphore

  return 0;
} //end main





