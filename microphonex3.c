#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>     // open
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <signal.h>
#include <pthread.h>


bool flag = true;
//void microphone_init(int handler, int address);
char *i2c = "/dev/i2c-1";
void sighandler(int sig){
  if(sig == SIGINT){
    flag = false;
  }else{
    flag = true;
  }
}

void *sound_funct(void *args);

struct i2c_args {
    int     address;
    char   *i2c_device;
}; 

//int ADS_address = 0x48;   // Address of our device on the I2C bus

int main(){
    pthread_t   sound_thread[3]; //create 3 threads for microphone
    int         iret[3];   
    
    //create functions arguments
    struct i2c_args *sound_args[3] = (struct i2c_args*) (malloc(sizeof(struct i2c_args)));
    sound_args[0]->address    = 0x48;             //bus address
    sound_args[0]->i2c_device = "/dev/i2c-1";     //i2c devices
    sound_args[1]->address    = 0x48;
    sound_args[1]->i2c_device = "/dev/i2c-3";
    sound_args[2]->address    = 0x48;
    sound_args[2]->i2c_device = "/dev/i2c-4";
    
     

    //create threads
    for(int i= 0; i<3;i++){
        if((iret[i]=pthread_create(&sound_thread[i], NULL, &sound_funct, (void *)sound_args[i]))){
            printf("Thread creation failed: %d\n", iret[i]);
        }
    }

    //wait till thread complete
    //three sound sensor compete to find the event
    //if a thread found the event first
    //stop other threads  and return with a location of the event
    for(int i= 0; i<3;i++){
        pthread_join(sound_thread[i],NULL);
    }
}

void *sound_funct(void *sound_args){
    int I2CFile;              //I2c handler
    uint8_t writeBuf[3];      // Buffer to store the 3 bytes that we write to the I2C device
    uint8_t readBuf[2];       // 2 byte buffer to store the data read from the I2C device
  
    int16_t val;              // Stores the 16 bit value of our ADC conversion

  
    I2CFile = open(((struct i2c_args*)sound_args)->i2c_device, O_RDWR);     // Open the I2C device
  
    ioctl(I2CFile, I2C_SLAVE, ((struct i2c_args*)sound_args)->address);   // Specify the address of the I2C Slave to communicate with

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

    //works for single conversion, but we use continuous mode this time
    // Wait for the conversion to complete, this requires bit 15 to change from 0->1
    // while ((readBuf[0] & 0x80) == 0)  // readBuf[0] contains 8 MSBs of config register, AND with 10000000 to select bit 15
    // {
    //     read(I2CFile, readBuf, 2);    // Read the config register into readBuf to update the conversion status
    // }

    writeBuf[0] = 0;                  // select conversion register 
    write(I2CFile, writeBuf, 1);
    float current = 0;
    float threshold = 1.6;
    while(1){
        read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf

        val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
        current = (float)val*4.096/32767.0;
        printf("Voltage Reading %f (V) \n", current);    // Print the result to terminal, first convert from binary value to mV
        // printf("percentage %f %% \n", percentage);
        float difference = current - threshold;
    //printf("current is: %f (V) , previous is: %f (V, different: %f) \n", current, previous, difference);
        sleep(0.5);
        if(difference<0){
        printf("Blow!\n");
    }

    if(!flag){
      close(I2CFile);
      exit(0);
    }
    //previous = current;
  }
        
  close(I2CFile);
}