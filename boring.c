// Project 1
// CS 3013
// Jeffrey Huang and Jyalu Wu

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

long int faults[2] = {0,0};

/*
 * Executes the command based on what number is specified
 * in the arguments. If it is given 0 it executes
 * "whoami", if it is given 1 it executes "last", and if
 * it's given 2 it executes "ls -ah /home".
 * @param command, an int representing the given command
 */
void execute(int command){
	struct rusage usage;
	struct timeval start,end;

    // init statistics
	gettimeofday(&start,NULL);
    getrusage(RUSAGE_SELF, &usage);
    faults[0] = usage.ru_majflt;
    faults[1] = usage.ru_minflt;

	printf("Running command: ");
	if(command == 0) printf("whoami\n");
	else if(command == 1) printf("last\n");
	else if(command == 2) printf("ls -al /home\n");
	int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        char *myargs[4];
        switch(command){
        	case(0): 
        		myargs[0] = strdup("whoami");
        		myargs[1] = NULL;
        		break;
        	case(1):  
        		myargs[0] = strdup("last");
        		myargs[1] = NULL;
        		break;
        	case(2): 
        		myargs[0] = strdup("ls");
        		myargs[1] = strdup("-al");
        		myargs[2] = strdup("/home");
        		myargs[3] = NULL;
        		break;
        }
        execvp(myargs[0], myargs);
        printf("this shouldn't print out");
    } else {
        while(wait(NULL)!=rc);
        getrusage(RUSAGE_SELF,&usage);
        gettimeofday(&end, NULL);
        printf("\n-- Statistics --\n");
        printf("Elapsed time: %ld millisecond(s)\n", (end.tv_usec - start.tv_usec) / 1000);
		faults[0] = usage.ru_majflt - faults[0];
		faults[1] = usage.ru_minflt - faults[1];
        printf("Page Faults: %ld\n", faults[0]);
        printf("Page Faults (reclaimed): %ld \n", faults[1]);
        printf("-- End of Statistics --\n\n");
    }
}


/*
 * Executes the following commands and prints out some
 * statistics corresponding to each child process:
 *      whoami, last, ls -al /home
 */
int main(int argc, char *argv[]){
	int i;
	for(i = 0; i < 3; i++) {
		execute(i);
	}
    return 0;
}

