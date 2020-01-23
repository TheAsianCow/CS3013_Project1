#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>
#include "custom.h"

long int faults[2] = {0,0};


/*
 * Parses ???
 */
void  parse(char *line, char **argv){
	/* if not the end of line ....... */ 
     while (*line != '\0') {
     
     	/* replace white spaces with 0    */
          while(*line == ' ' || *line == '\t' || *line == '\n') *line++ = '\0';
          /* save the argument position     */
          *argv++ = line;          
          /* skip the argument until ...    */
          while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n') line++;             
     }
     /* mark the end of argument list  */
     *argv = '\0';                 
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

int main(int argc, char *argv[]) {
	char* file_path = "custom.txt";
    char* line; 
    ssize_t size;
    size_t n = 0;
    printf("Hello this is Minh Anh being a CS major hehehehehheheh\n");
    
	char* currentDir = "/";

	printDir(currentDir_ptr);

	char* tempDir= "/home/lu/Documents";

	printf("Want to change to: %s\n", tempDir);
	currentDir_ptr = tempDir;
	changeDir(currentDir_ptr);
	printDir(currentDir_ptr);


	// Parsing file
	// FILE* file = fopen(file_path,"r");
 //    size = getline(&line,&n,file);
 //    while(size >=0){
 //        // printf("Line read: %s\n",line);
 //        // execute()
 //        size = getline(&line,&n,file);
 //    }


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
