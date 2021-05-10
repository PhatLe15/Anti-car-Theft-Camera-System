

#ifndef MICROPHONE_H
#define MICROPHONE_H

extern sem_t mutex;
extern int pos = -1;
extern bool taken = false;

void sighandler(int sig);
void *sound_funct(char* i2c_device);


#endif
