#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>     // open
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <pthread.h>      //create 3 threads for sound sensor
#include <semaphore.h>    //share resource between each thread
#include <signal.h>

#include "microphone.h"


bool flag = true;
//bool taken = false;
//pos = -1;
//sem_t mutex;

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

void *sound_funct(char* i2c_device)
{
  int I2CFile;         //I2c handler
  uint8_t writeBuf[3]; // Buffer to store the 3 bytes that we write to the I2C device
  uint8_t readBuf[2];  // 2 byte buffer to store the data read from the I2C device

  int16_t val; // Stores the 16 bit value of our ADC conversion

  int16_t EMA_S_low = 0;    
  int16_t EMA_S_high = 0;
  float EMA_a_low = 0.32;  //cutoff frequency coefficient
  float EMA_a_high = 0.75;
  int16_t bandpass = 0;   //sound data after filtered 
  int16_t highpass = 0;
  int t = 0;
  int max= 0;

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

  /* Wait for the conversion to complete, this requires bit 15 to change from 0->1
  // while ((readBuf[0] & 0x80) == 0)  // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
  // {
  //     read(I2CFile, readBuf, 2);    // Read the config register into readBuf to update the conversion status
   }*/

  writeBuf[0] = 0;                  // select conversion register 
  write(I2CFile, writeBuf, 1);

  while(1){
    if(taken){  //if one of the sensor found the sound stop reading and end the sampling process
      break;

    }else{ //if nothing found yet
          read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf
    val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
    printf("binary val: %d\n",val);
    

    EMA_S_low = (EMA_S_low*val) + ((1-EMA_a_low)*EMA_S_low);     //lower bound
    EMA_S_high = (EMA_S_high*val) + ((1-EMA_a_high)*EMA_S_high); //upper bound

    //highpass = val - EMA_S_high;
    bandpass = EMA_S_high- EMA_S_low;  //the input after filter

    if(bandpass>max){ //detect the peak within the speaktrum
      max = bandpass;
    }
    
    if(max<20 && max > 12){ //if the peak fall within threshold, it might be a breaking event
       sem_wait(&mutex);  //obtain semaphore to lock the other
      while(t < 2000){ //within 2s
        //continue reading input
        read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf
        val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
        
        //go through high pass filter
        EMA_S_high = (EMA_S_high*val) + ((1-EMA_a_high)*EMA_S_high); 
        highpass = val - EMA_S_high;
        
        if( highpass > 3000 && highpass < 5000){ // if the low frequency is within this threshold, continue sampling
          t = t + 1;
        } else{
            break;
        }
        
        if(t = 1999){ //if the value consistence then this is the breaking event
          if(!taken){ //in case other thread already detect the event in this stage
          taken = true;   //claim the first event detected

          //choose the position and claim the throne :)
            if(i2c_device == "/dev/i2c-1"){
              pos = 1;
            }else if(i2c_device == "/dev/i2c-3"){
              pos = 2;
            }else{
              pos = 3;
            }
            break;//out of the loop
          }
        }  
      }
    }else{ //reset the peak and look for other peak
        max = 0;
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


//sample main for unit testing
/*int main() {
  signal(SIGINT, sighandler);
  int ADS_address = 0x48;   // Address of our device on the I2C bus
  int I2CFile; //I2c handler
  
  uint8_t writeBuf[3];      // Buffer to store the 3 bytes that we write to the I2C device
  uint8_t readBuf[2];       // 2 byte buffer to store the data read from the I2C device
  
  int16_t val;              // Stores the 16 bit value of our ADC conversion
  
  I2CFile = open(i2c, O_RDWR);     // Open the I2C device
  
  ioctl(I2CFile, I2C_SLAVE, ADS_address);   // Specify the address of the I2C Slave to communicate with

  //microphone_init(I2CFile, ADS_address);


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
  float current = 0;
  int16_t threshold = 13000;
  while(1){
    //difference = 0;
    read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf

    val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
    //printf("binary val: %d\n",val);
    
    //current = (float)val*4.096/32767.0;
    //printf("Voltage Reading %f (V) \n", current);    // Print the result to terminal, first convert from binary value to mV

    //check for event
    int16_t difference = val - threshold;
    if(difference<0){
      printf("I2c-3:Blow!\n");
      difference = 0;
    }

    //send signal to stop the process
    if(!flag){
      close(I2CFile);
      exit(0);
    }
    //previous = current;
    //sleep(0.5);//sampling every 0.5 second (reduce for faster sampling)
  }
        
  close(I2CFile);
  
  return 0;

}

*/
