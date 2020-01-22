#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

void execute(int command){
	struct rusage usage;
	struct timeval start,end;
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
        getrusage(RUSAGE_SELF,&usage);
        gettimeofday(&start,NULL);
        execvp(myargs[0], myargs);
    } else {
        // int wc = wait(NULL);
        while(wait(NULL)!=rc);
        getrusage(RUSAGE_SELF,&usage);
        gettimeofday(&end, NULL);
        printf("\n-- Statistics --\n");
        printf("Elapsed time: %ldus\n", end.tv_usec - start.tv_usec);
        printf("Page Faults: \n");
        printf("Page Faults (reclaimed): \n");
        printf("-- End of Statistics --\n\n");
    }
}

int main(int argc, char *argv[]){
	int i;

	for(i = 0; i < 3; i++) execute(i);
    return 0;
}

