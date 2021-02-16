#include <stdlib.h>
#include <stdio.h>        
#include <stdbool.h>      //bool
#include <unistd.h>
#include <sys/ioctl.h>     //i2c control
#include <fcntl.h>         // open
#include <inttypes.h>      // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <signal.h>        //stop programm with kernel signal
#include <pthread.h>      //create 3 threads for sound sensor
#include <semaphore.h>    //share resource between each thread
#include <wiringPi.h>     // set gpio pin for servo
#include <softPwm.h>      // send PWM signal to servo

#include <string.h>
#include <time.h> 

static pid_t pid = 0;
bool taken = false;
bool flag = true;
char *i2c_sound[3] = {"/dev/i2c-1","/dev/i2c-3","/dev/i2c-4"};
int who_first = 0;
sem_t mutex;
int pos = -1;

void sighandler(int sig);
void *sound_funct();
int servo_position(int position);
void startVideo(char *filename, char *options);
void stopVideo(void);


int main()
{
  int iret[3];
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
  if(who_first==1){
    printf("Move servo to position %u\n", pos);
  }else if(who_first==2){
    printf("Move servo to position %u\n", pos);
  }else if(who_first == 3){
    printf("Move servo to position %u\n", pos);
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
void *sound_funct(char* i2c_device)
{
  int I2CFile;         //I2c handler
  uint8_t writeBuf[3]; // Buffer to store the 3 bytes that we write to the I2C device
  uint8_t readBuf[2];  // 2 byte buffer to store the data read from the I2C device

  int16_t val; // Stores the 16 bit value of our ADC conversion

  I2CFile = open(i2c_device, O_RDWR); // Open the I2C device

  ioctl(I2CFile, I2C_SLAVE, 0x48); // Specify the address of the I2C Slave to communicate with

  // These three bytes are written to the ADS1115 to set the config register and start a conversion 
  writeBuf[0] = 1;          // let pointer register to select config reg

  //edit config register
  writeBuf[1] = 0xC2;       // This sets the 8 MSBs of the config register (bits 15-8) to 11000010  
  writeBuf[2] = 0x03;       // This sets the 8 LSBs of the config register (bits 7-0) to 00000011
      
  // Write writeBuf to the ADS1115, the 3 specifies the number of bytes we are writing,
  // this begins a single conversion
  write(I2CFile, writeBuf, 3);  

  // Initialize the buffer used to read 16bit data from the ADS1115 to 0
  readBuf[0]= 0;        
  readBuf[1]= 0;

  // Wait for the conversion to complete, this requires bit 15 to change from 0->1
  // while ((readBuf[0] & 0x80) == 0)  // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
  // {
  //     read(I2CFile, readBuf, 2);    // Read the config register into readBuf to update the conversion status
  // }

  writeBuf[0] = 0;                  // select conversion register 
  write(I2CFile, writeBuf, 1);

  int16_t threshold = 13000;
  int16_t difference = 0;
  while(1){
    if(taken){  //if one of the sensor found the sound stop reading and end the sampling process
      break;

    }else{ //if nothing found yet
      read(I2CFile, readBuf, 2);            // Read the contents of the conversion register into readBuf
      val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
    //printf("binary val: %d\n",val);
    
    //convert to voltage value if needed
    //current = (float)val*4.096/32767.0;
    //printf("Voltage Reading %f (V) \n", current);    // Print the result to terminal, first convert from binary value to mV

    //check for event
    difference = val - threshold; //event condition

    if(difference<0){  //if blow
      //printf("blow 1");
      sem_wait(&mutex);  //obtain semaphore to lock the other
      if(!taken){ //in case other thread already detect the event in this stage
        taken = true;   //claim the first event
        //printf("%s:Blow!\n", i2c_device);

        //choose the position and claim the throne :)
        if(i2c_device == "/dev/i2c-1"){
          who_first = 1;
          pos = 1;
        }else if(i2c_device == "/dev/i2c-3"){
          who_first = 2;
          pos = 2;
        }else{
          who_first = 3;
          pos = 3;
        }
      }
      sem_post(&mutex); //unlock semaphore
      break;
    }
    

    //send signal to terminate the entire process
    if(!flag){
      close(I2CFile);
      exit(0);
    }
    //previous = current;
    sleep(0.5);//sampling every 0.5 second (reduce for faster sampling)
    }
    
  }
        
  close(I2CFile); //close I2C bus
  
  return 0;
} //end sound_funct()

//send terminate signal in case program did not stop properly
void sighandler(int sig)
{
  if (sig == SIGINT)
  {
    flag = false;
  }
  else
  {
    flag = true;
  }
}

//move servo to a specific location
int servo_position(int position){ //0 25 90 135 or 180 degree
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
}




