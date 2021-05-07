#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>      
#include "camera.h"

static pid_t pid = 0;
static pid_t pid2 = 0;
char *cmdEcho[] = { "/bin/echo", "Location of the Event", 0 };
char *cmdMail[] = { "/usr/bin/mail", "-s","Breaking Alert!!!","rasp24@gmail.com", 0 };

void runpipe(int pfd[]);  //pass command and rund the pipe

void sendEmail(char *detectAt/*, char *optionsEcho, char *optionsMail*/) {
        cmdEcho[1] = detectAt;
        int pid, status;
        int fd[2];

        pipe(fd); //create a pipe 

        switch(pid=fork()){
            case 0: //child
                runpipe(fd);
                exit(0);
            default: //parent
                while ((pid = wait(&status)) != -1)
			        fprintf(stderr, "process %d exits with %d\n", pid, WEXITSTATUS(status));
		        break;
            case -1:
		        perror("fork");
		        exit(1);
        }
        exit(0);
}



void runpipe(int pfd[]){
	switch (pid2 = fork()) {
    	case 0: /* child */
	    	dup2(pfd[0], 0);
		    close(pfd[1]);	/* the child does not need this end of the pipe */
		    execvp(cmdMail[0], cmdMail);  //execute mail command
		    perror(cmdMail[0]); 

	    default: /* parent */
    		dup2(pfd[1], 1);
		    close(pfd[0]);	/* the parent does not need this end of the pipe */
		    execvp(cmdEcho[0], cmdEcho);    //execute echo command
		    perror(cmdEcho[0]);

	    case -1:
		    perror("fork");
		    exit(1);
	}
}

