#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>
#include "custom.h"
// #include <libexplain/gcc_attributes.h>

long int faults[2] = {0,0};


/*
 * 
 */
void parse(char* line, char** args){
     const char s[2] = " ";
     char* token = strtok(line, s);
     int argc = 0;
     while(token!=NULL){
        args[argc] = strdup(token);
        argc++;
        token = strtok(NULL, s);
     }
     args[argc] = NULL;
     // for(int i = 0; i < argc; i++) printf("arg: %s\n",args[i]);
}

/*
 * int* faults: [majFaults, minFaults]
 */
void execute(char* command){
    struct rusage usage;
    struct timeval start,end;

    gettimeofday(&start,NULL);
    printf("Running command: %s\n",command);
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
        // printf("passing command: %s\n", myargs[0]);
        execvp(myargs[0], myargs);
        // fprintf(stderr,"execvp error: %s\n",explain_execvp(myargs[0], myargs));
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

int main(int argc, char *argv[]) {
	char* file_path = "custom.txt";
    char* line; 
    ssize_t size;
    size_t n = 0;
    
	// char* currentDir = "/home";
	// char* currentDir_ptr = currentDir;

	// printDir(currentDir_ptr);

	// char* tempDir= "/home/lu/Documents";

	// printf("Want to change to: %s\n", tempDir);
	// currentDir_ptr = tempDir;
	// changeDir(currentDir_ptr);
	// printDir(currentDir_ptr);


	FILE* file = fopen(file_path,"r");
    size = getline(&line,&n,file);
    while(size >=0){
        // printf("Line read: %s\n",line);
        if(line[size-1]=='\n') line[size-1]='\0';
        // printf("executing line from file: %s\n",line);
        execute(line);
        size = getline(&line,&n,file);
    }

    return 0;
}


/*
 * Changes the current directory for this current process
 * as well as for any children processes. Called when
 * "ccd" is parsed.
 */
void changeDir(char* newDir) {
	if (chdir(newDir) != 0) {
		perror(("Error changing the directory to %s\n", newDir));

	}
	/*
	save newDir as global or return or something idk
	call changeDir again when new child born
	*/
}


/*
 * Prints out the name of the current working directory.
 */
void printDir(char* currentDir) {
	getcwd(currentDir, sizeof(currentDir));
	printf("Current directory: %s\n", currentDir);
}
