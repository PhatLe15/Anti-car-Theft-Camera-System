#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>      
#include "camera.h"

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

/*void initVideo(void) {

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
}*/
