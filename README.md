# Anti-car-Theft-Camera-System

![sdfg](https://lh5.googleusercontent.com/B0WokYW12llLMfzxbnLCZsedYxVLgoPfLHYIQxSOqiSRXHG_ViFwIWeDy3ORqUNTv505ZbMPyGT7PE1_Ny7itRbihwW0ctNHGaOMevV0rR9DLmW0Z6l7alKIDWd0yBieNtUe8GuL)

***Link to Video Demo***: [Click here](https://drive.google.com/file/d/1tRAcBPpMMIhOE4YJsMl3Cke628F7knyF/view?usp=sharing)
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
  - [Setup and build instruction](#setup-and-build-instruction)
    - [Install Libraries and packages](#install-libraries-and-packages)
    - [System Configuration](#system-configuration)
      - [I2C](#i2c)
      - [SMTP(send email)](#smtpsend-email)
      - [Video Directory](#video-directory)
    - [Compile code with gcc](#compile-code-with-gcc)
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
* [x] Detect glass-breaking sound
  * [x] Using bandpass filter
  * [ ] Using Machine learning
* [x] Send alert through email
  * [x] Location of the event
  * [ ] Compress and attach video

The following **additional** features are implemented:
- [x] Put everything in a case

- [x] Machine Learning Motion tracking after breaking is detected
  - [x] Unit Implementation
  - [ ] Integrate with the system
 
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

## Setup and build instruction
### Install Libraries and packages
Before starting the program, we need to download necessary Libraries and Packages using the command lines as follow:

```
sudo apt-get install msmtp msmtp-mta
sudo apt-get install wiringpi
```
****Note that the Raspian Pi is currently updated and I2c and Camera feature is enable in the setting***

### System Configuration
#### I2C
The first step to run the program is to configure the Linux system files. Since the `GPIO` only have one **I2C** bus. We can add extra content of the file under the directory `root/config.txt` as follow:
```
dtoverlay=i2c-gpio,bus=4,i2c_gpio_delay_us=1,i2c_gpio_sda=23,i2c_gpio_scl=24
dtoverlay=i2c-gpio,bus=3,i2c_gpio_delay_us=1,i2c_gpio_sda=17,i2c_gpio_scl=27
```
#### SMTP(send email)
Then, we need to set up a local SMTP server to communicate with Gmail service by editing the file under the path /etc/msmtprc as follow:

```
defaults
auth	on
tls	on
tls_trust_file	/etc/ssl/certs/ca-certificates.crt
logfile	~/.msmtp.log

account	gmail
host	smtp.gmail.com
port	587

from	rasppi24@gmail.com
user	rasppi24@gmail.com
password	EmailPassword

account default:	gmail
```
****Note that we need to use an actual email account and password.***

#### Video Directory
To select the directory where the evident video is stored, we can change the path inside the main function at `main.c` as follow:
```
char directory[1000] = "/home/pi/Desktop/HelloWorld/videoFolder/"; //set directory where video is saved
```

### Compile code with gcc
After all of the above steps are done, we can go ahead and build the main file program using the below command:
```
sudo gcc -o microphone main.c camera.c email.c microphone.c servo.c -lwiringPi -lpthread -lrt
sudo ./microphone
```

[Back To The Top](#Anti-car-Theft-Camera-System)

---

## References

- Wiring Pi - [WiringPi](http://wiringpi.com)
- Controlling the Raspberry Pi camera from C - [Ceptimus](http://ceptimus.co.uk/?p=91)
- RPi and I2C Analog-Digital Converter - [University of Cambridge OpebLabTools](http://openlabtools.eng.cam.ac.uk/Resources/Datalog/RPi_ADS1115/)
- Consumer Industrial: Acoustic Glass Break Detector - [Vadym, G. (2019, Dec 12)](https://www.cypress.com/file/141276/download)
- Arduino Tutorial: simple High-pass, Band-pass and Band-stop Filtering - [Aasvik, M (2016, Mar 10)](https://www.norwegiancreations.com/2016/03/arduino-tutorial-simple-high-pass-band-pass-and-band-stop-filtering/)

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


