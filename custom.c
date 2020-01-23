#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>


//
void changeDir() {

}

// cpwd
void printCurrentDir() {
	
}
long int faults[2] = {0,0};

void parse(char* line, char** args){
     const char s[2] = " ";
     char* token = strtok(line, s);
     int argc = 0;
     while(token!=NULL){
        args[argc] = strdup(token);
        argc++;
        token = strtok(NULL, s);
     }
}

/*
 * int* faults: [majFaults, minFaults]
 */
void execute(char* command){
    struct rusage usage;
    struct timeval start,end;

    gettimeofday(&start,NULL);
    printf("Running command: %s",command);
    int rc = fork();
    if (rc < 0) {
        // fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        char *myargs[34];
        myargs[33] = NULL;
        parse(command, myargs);
        getrusage(RUSAGE_SELF,&usage);
        //gettimeofday(&start,NULL);
        execvp(myargs[0], myargs);
        printf("this shouldn't print out");
    } else {
        // int wc = wait(NULL);
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

int main(int argc, char *argv[]){
	char* file_path = "custom.txt";
    char* line; 
    ssize_t size;
    size_t n = 0;
    FILE* file = fopen(file_path,"r");
    size = getline(&line,&n,file);
    while(size >=0){
        // printf("Line read: %s\n",line);
        // execute()
        size = getline(&line,&n,file);
    }
    fclose(file);

    return 0;
}