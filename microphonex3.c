#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>         // open
#include <inttypes.h>      // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <signal.h>
#include <pthread.h>

bool flag = true;
//void microphone_init(int handler, int address);
char *i2c_sound1 = "/dev/i2c-1";
char *i2c_sound2 = "/dev/i2c-3";
char *i2c_sound3 = "/dev/i2c-4";
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

void *sound_funct();

// struct i2c_args
// {
//   int address;
//   char *i2c_device;
// };

//int ADS_address = 0x48;   // Address of our device on the I2C bus

int main()
{
  pthread_t sound_thread[3]; //create 3 threads for microphone
  int iret[3];

  // //create functions arguments
  // struct i2c_args *sound_args[3] = (struct i2c_args*) (malloc(sizeof(struct i2c_args)));
  // sound_args[0]->address    = 0x48;             //bus address
  // sound_args[0]->i2c_device = "/dev/i2c-1";     //i2c devices
  // sound_args[1]->address    = 0x48;
  // sound_args[1]->i2c_device = "/dev/i2c-3";
  // sound_args[2]->address    = 0x48;
  // sound_args[2]->i2c_device = "/dev/i2c-4";

  //create threads
  if(iret[0]=pthread_create(&sound_thread[0], NULL, &sound_funct, i2c_sound1)){
    printf("Thread creation failed: %d\n", iret[0]);
  }
    
  if(iret[1]=pthread_create(&sound_thread[1], NULL, &sound_funct, i2c_sound2)){
    printf("Thread creation failed: %d\n", iret[1]);
  }
  
  if(iret[2]=pthread_create(&sound_thread[2], NULL, &sound_funct,i2c_sound3)){
    printf("Thread creation failed: %d\n", iret[2]);
  }

    //wait till thread complete
    //three sound sensor compete to find the event
    //if a thread found the event first
    //stop other threads  and return with a location of the event
    for(int i= 0; i<3;i++){
      pthread_join(sound_thread[i], NULL);
    }
}

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

    // Initialize the buffer used to read data from the ADS1115 to 0
  readBuf[0]= 0;        
  readBuf[1]= 0;

  // Wait for the conversion to complete, this requires bit 15 to change from 0->1
  // while ((readBuf[0] & 0x80) == 0)  // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
  // {
  //     read(I2CFile, readBuf, 2);    // Read the config register into readBuf to update the conversion status
  // }

  writeBuf[0] = 0;                  // select conversion register 
  write(I2CFile, writeBuf, 1);

  int16_t threshold = 10000;
  while(1){
    read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf

    val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
    //printf("binary val: %d\n",val);
    
    //current = (float)val*4.096/32767.0;
    //printf("Voltage Reading %f (V) \n", current);    // Print the result to terminal, first convert from binary value to mV

    //check for event
    int16_t difference = val - threshold;
    if(difference<0){
      printf("%s:Blow!\n", i2c_device);
      difference = 1;
      sleep(1);
    }

    //send signal to stop the process
    if(!flag){
      close(I2CFile);
      exit(0);
    }
    //previous = current;
    sleep(0.5);//sampling every 0.5 second (reduce for faster sampling)
  }
        
  close(I2CFile);
  
  return 0;

  close(I2CFile);
}