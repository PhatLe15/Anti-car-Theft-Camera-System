# Anti-car-Theft-Camera-System

![sdfg](https://lh5.googleusercontent.com/B0WokYW12llLMfzxbnLCZsedYxVLgoPfLHYIQxSOqiSRXHG_ViFwIWeDy3ORqUNTv505ZbMPyGT7PE1_Ny7itRbihwW0ctNHGaOMevV0rR9DLmW0Z6l7alKIDWd0yBieNtUe8GuL)


### Table of Contents

- [Anti-car-Theft-Camera-System](#anti-car-theft-camera-system)
    - [Table of Contents](#table-of-contents)
    - [Team Members](#team-members)
  - [Description](#description)
    - [Software Tools](#software-tools)
    - [Hardware Tools](#hardware-tools)
    - [Cost and Budget](#cost-and-budget)
  - [Requirements](#requirements)
  - [Schematics](#schematics)
  - [Design methodology](#design-methodology)
    - [main program](#main-program)
    - [Sound detection thread function](#sound-detection-thread-function)
  - [Source Code Reference](#source-code-reference)
    - [Compile code with gcc](#compile-code-with-gcc)
    - [camera.c](#camerac)
    - [microphone.c](#microphonec)
    - [servo.c](#servoc)
  - [References](#references)
  - [License](#license)
  - [Author Info](#author-info)

### Team Members

- Phat Le
- Archie Macabeo
- Thien Hoang
- Katt Owens
- ***Professor Advisor*** : Wo-tak Wu

[Back To The Top](#Anti-car-Theft-Camera-System)

---

## Description

An Embedded System that can detect glass breaking sound which trigger camera recording and send notification alert.

This project was inspired by my living situation inside of a fenced community. Inside is relatively safe, but due to crowding, many are forced to park on the street outside the fence. With some regularity, car burglars will break into these cars at night and steal items. Thus this project’s goal and ultimate requirement is to either catch them in the act and lead to their arrests or, more optimally, dissuade them from stealing. It would provide more security and peace of mind to the residents and perhaps increase property values.

### Software Tools

- Raspian Linux
- WiringPi
- PWM
- I2C
- Raspivid
- Threads/Child fork
- Mutex/Conditional variable

### Hardware Tools

- Raspberry Pi **3B+**  - [RaspberryPi Website](https://www.raspberrypi.org/products/raspberry-pi-3-model-b-plus/)
- Microphone **KY-037** - [Datasheet](https://datasheet4u.com/datasheet-pdf/Joy-IT/KY-037/pdf.php?id=1402047)
- Servo Motor **SG90**  - [Datasheet](http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf)
- Analog-Digital Converter **ADS1115** - [Datasheet](https://www.ti.com/lit/ds/symlink/ads1115.pdf?ts=1612287514232&ref_url=https%253A%252F%252Fwww.google.com%252F)
- Pi Camera **Rev 1.3** - [Camera Module Spec](https://www.raspberrypi.org/documentation/hardware/camera/)

### Cost and Budget
The initial budget plan for the project was less than or equal to `$150`. Since some components can only be purchased in bulk or have been damaged during the project development period, the project might have been over spent for extra parts. However, the current spending still fall below our budget.

| Model  | Quantity |      Price(USD)      |
|:-------------------:|:-------:|:--------------------------------------------------------------------------------:|
| Rasberry Pi 3B+ | 1 | 39.99$ |
| ADC ADS1115 | 3 | 11.99$ |
| Microphone KY-037 | 3 | 13.99$ |
| Pi Camera Rev 1.3 | 1 | 12.99$ |
| Servo Motor SG90 | 4 | 8.99$ |
| Case and Jumper Wires | 1 | 7.49$ |
| Soldering Tool | 1 | 10.99$ |
| **Total** |  | 106.43$ |


[Back To The Top](#Anti-car-Theft-Camera-System)

---

## Requirements

The following **required** functionality is complete:

* [x] Control servo motors to the desinated position
* [x] Record video and save to local folder with Date naming
* [x] Sampling analog signal from sound sensor
  * [x] Use ADC to convert analog from sensor to gpio digital signal
  * [x] Implement I2C to communicate with ADC  
* [x] Sampling from three separate microphone in parallel
    * [x] Run from different thread
    * [x] Each thread communicate from different I2C bus
* [x] Use semaphore to manage the share thread resources 
* [x] Move camera to the position where the event occur

The following **future improvement** features are implemented:
* [ ] Detect glass-breaking sound
* [ ] Send alert through SMS

The following **additional** features are implemented:
- [ ] Put everything in a case

- [ ] Create UI Web server that can see live streaming and control from device. 
- [ ] Install night vision module to camera
 
[Back To The Top](#Anti-car-Theft-Camera-System)

## Schematics
Since the Raspberry Pi GPIO is not capable of receiving `analog` signal. The microphone sensors will have to go through Analog-to-digital Converter(`ADS1115`) modules which is then communicate with the Raspberry Pi using separate I2C buses to improve throughput.

![sdf](https://github.com/PhatLe15/Anti-car-Theft-Camera-System/blob/main/Schematic.png?raw=true)

## Design methodology
The initial camera system system is divided into 5 different states as shown in ***Figure*** below. Specifically, the system will not begin its detection and stay in `IDLE` mode if a `START` signal is not asserted. If `START` asserted, the sound sampling and detecion begin in `SOUND DETECTION` mode. The device will continue on this mode until the glass breaking sound is detected that will send a `FOUND` signal to move to the next state. The camera is moved by a servo motor in `MOVE SERVO` mode, then, record a video in the `VIDEO RECORDING` mode. 


![sdf](https://github.com/PhatLe15/Anti-car-Theft-Camera-System/blob/main/FSM.png?raw=true)

### main program
The ***Figure*** below illustrate the design flow of the **main.c** program. The program will first create `3 threads` of sound sampling since it need to actively sampling and analysing the sound in parallel from `3 angles`. When the first thread detect the event, the program will decide that is the angle the servo need to move to and start recording the video. Then, it go back to thread creating and sampling state. 

![sdf](https://github.com/PhatLe15/Anti-car-Theft-Camera-System/blob/main/Main%20Program%20Flow%20Chart.png?raw=true)

### Sound detection thread function
For the sound detection which is running from 3 different threads, each thread will share a `Found` variable which is protected by `POSIX Semaphore`. If a thread found the event, it will update the global variable position `pos`, and end all threads by aserting the `Found` variable. 

![sdf](https://github.com/PhatLe15/Anti-car-Theft-Camera-System/blob/main/Sound%20sensor%20Flow%20Chart.png?raw=true)

## Source Code Reference

### Compile code with gcc
```
sudo gcc -o microphone microphonex3.c -lwiringPi -lpthread -lrt
sudo ./microphone
```

### camera.c
```c
    static pid_t pid = 0;

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

int main(int argc, char **argv) {

    //get current time as name of file and combine with the directory
    time_t now;
    time(&now);
    
    //set directory
    char directory[1000] = "/home/pi/Desktop/HelloWorld/videoFolder/";
    
    //create name by real time 
    strcat(directory,ctime(&now));
    strcat(directory,".h264");

    printf("Recording video for 5 secs...");
    startVideo(directory, "-cfx 128:128 -rot 180"); 
    fflush(stdout);
    sleep(5);//change recording time here
    stopVideo();
    printf("\nVideo stopped - exiting in 2 secs.\n");
    sleep(2);
    return 0;
}
```

### microphone.c
```c
bool flag = true;

char *i2c = "/dev/i2c-1";
void sighandler(int sig){
  if(sig == SIGINT){
    flag = false;
  }else{
    flag = true;
  }
}

int main() {
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
  float threshold = 1.6;
  while(1){
    read(I2CFile, readBuf, 2);        // Read the contents of the conversion register into readBuf

    val = readBuf[0] << 8 | readBuf[1];   // Combine the two bytes of readBuf into a single 16 bit result
    current = (float)val*4.096/32767.0;
    printf("Voltage Reading %f (V) \n", current);    // Print the result to terminal, first convert from binary value to mV
    // printf("percentage %f %% \n", percentage);
     float difference = current - threshold;
    //printf("current is: %f (V) , previous is: %f (V, different: %f) \n", current, previous, difference);
    
    sleep(0.5);//sampling every 0.5 second (reduce for faster sampling)
    if(difference<0){
      printf("Blow!\n");
    }

    //send signal to stop the process
    if(!flag){
      close(I2CFile);
      exit(0);
    }
    //previous = current;
  }
        
  close(I2CFile);
  
  return 0;

}
```

### servo.c

```c
int servo_position(int position);

int main(void){
  wiringPiSetupGpio();
  
  pinMode(17,PWM_OUTPUT);

  softPwmCreate(17,servo_position(2),100);
  delay(500);

  softPwmWrite(17,servo_position(0));
  delay(500);

  softPwmWrite(17,servo_position(1));
  delay(500);

    softPwmWrite(17,servo_position(2));
  delay(500);

  softPwmWrite(17,servo_position(3));
  delay(500);

  softPwmWrite(17,servo_position(4));
  delay(500);

  softPwmStop(17);
  
  return 0;
}

int servo_position(int position){ //0 25 90 135 180
  int pulse;
  switch (position)
  {
    case 0:
      pulse = 1;
      break;
    case 1:
      pulse = 5;
      break;
    case 2:
      pulse = 9.5;
      break;
    case 3:
      pulse = 15;
      break;
    case 4:
      pulse = 21;
      break;
    default:
      pulse = 1;
      break;
    return pulse;
  }
}
```
[Back To The Top](#read-me-template)

---

## References

- Wiring Pi - [WiringPi](http://wiringpi.com)
- Controlling the Raspberry Pi camera from C - [Ceptimus](http://ceptimus.co.uk/?p=91)
- RPi and I2C Analog-Digital Converter - [University of Cambridge OpebLabTools](http://openlabtools.eng.cam.ac.uk/Resources/Datalog/RPi_ADS1115/)

[Back To The Top](#Anti-car-Theft-Camera-System)

---

## License

MIT License

Copyright (c) 2021 Phat Le

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

[Back To The Top](#Anti-car-Theft-Camera-System)

---

## Author Info

- Github - [PhatLe15](https://github.com/PhatLe15)
- Linkedin - [phat-tan-le](https://www.linkedin.com/in/phat-tan-le/)
- Email - [phat.le@sjsu.edu]()


[Back To The Top](#Anti-car-Theft-Camera-System)


