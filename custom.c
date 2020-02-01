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
#include <getopt.h>
#include "custom.h"
// #include <libexplain/gcc_attributes.h>

long int faults[2] = {0,0};


/*
 * Parses the given line and splits it into separate strings
 * whenever it sees a space.
 * @param line, the given line
 * @param args, where it stores the split strings
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
}

/*
 * Executes the given command in the given directory. If ccd
 * or cpwd are called, it processes those commands internally.
 * @param command, the given command
 * @param currentDir_ptr, a pointer to the given directory
 */
void execute(char* command, char** currentDir_ptr){
    struct rusage usage;
    struct timeval start,end;

    gettimeofday(&start,NULL);
    getrusage(RUSAGE_SELF,&usage);
    printf("Running command: %s\n",command);
    char *myargs[34];
    myargs[33] = NULL;
    parse(command, myargs);
    if(strcmp(myargs[0],"ccd")==0){
        changeDir(&myargs[1]);
        *currentDir_ptr = myargs[1];
        printDir(currentDir_ptr);
    }
    else if(strcmp(myargs[0],"cpwd")==0){
        printDir(currentDir_ptr);
    }
    else{
        changeDir(currentDir_ptr);
        int rc = fork();
        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) {
            execvp(myargs[0], myargs);
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
}

/*
 * Parses through "custom.txt" and executes each line in the
 * text file. If the directory is changed, it remembers which
 * directory it was changed to and uses that directory in
 * future child processes.
 * @param argc, the number of arguments in the command line
 * @param argv, the arguments in the command line
 */
int main(int argc, char *argv[]) {
	char* file_path = "custom.txt";
    char* line; 
    ssize_t size;
    size_t n = 0;
    
    char arr[LINE_MAX];
	char* currentDir = arr;
    char** currentDir_ptr = &currentDir;

    // get the current working directory
    getcwd(arr, sizeof(arr));

    // parsing
	FILE* file = fopen(file_path,"r");
    size = getline(&line,&n,file);
    while(size >=0){
        if(line[size-1]=='\n') line[size-1]='\0';
        execute(line, currentDir_ptr);
        size = getline(&line,&n,file);
    }
    return 0;
}


/*
 * Changes the current directory to the given directory for
 * this current process as well as for any children processes.
 * Called when "ccd" is parsed.
 * @param newDir_ptr, a pointer to the new directory
 */
void changeDir(char** newDir_ptr) {
	if (chdir(*newDir_ptr) != 0) {
		perror(("Error changing the directory to %s\n", *newDir_ptr));

	}
}


/*
 * Prints out the name of the current working directory.
 * @param currentDir_ptr, a pointer to the current directory
 */
void printDir(char** currentDir_ptr) {
	getcwd(*currentDir_ptr, sizeof(*currentDir_ptr));
	printf("Current directory: %s\n\n", *currentDir_ptr);
}
