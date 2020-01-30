#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>
#include "multi.h"
// #include <libexplain/gcc_attributes.h>

long int faults[2] = {0,0};
char* bg_running[100];
int bg_cnt = 0;
int end_of_file_flag = 0;
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
}

/*
 * int* faults: [majFaults, minFaults]
 */
void execute(char* command, char** currentDir_ptr, int lineNum, 
        int* bgIndex_ptr, int** bg_ptr){
    struct rusage usage;
    // struct rusage wait3Usage;
    struct rusage* usage_ptr = &usage;
    // struct rusage* wait3Usage_ptr = &wait3Usage;
    struct timeval start,end;
    int childExitStatus = 0;
    int* childExitStatus_ptr = &childExitStatus;
    pid_t wait3Return = 0;

    gettimeofday(&start,NULL);
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
    else if(strcmp(myargs[0],"cproclist")==0){
        cproclist();
    }
    else{
        changeDir(currentDir_ptr);
        int rc = fork();
        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        } 
        // child
        else if (rc == 0) {
            printf("in child running command %s\n",command);
            getrusage(RUSAGE_SELF,&usage);
            execvp(myargs[0], myargs);
            // printf("child done\n");
        } 
        // parent
        else {
            printf("in parent of %d\n", rc);
            // BACKGROUND CHECK
            // printf("parent started\n");
            // printf("line num: %d\n", lineNum);
            // printf("bg ptr: %d\n", *bg_ptr[*bgIndex_ptr]);
            if (*bg_ptr[*bgIndex_ptr] == lineNum) {
                printf("is background\n");
                bg_running[bg_cnt] = strdup(command);
                bg_cnt++;
                // while(wait3(childExitStatus_ptr, WNOHANG, usage_ptr)){
                //     // printf("wait3 returned: %d\n", wait3Return);
                //     // printf("child exit status ptr: %d\n", *childExitStatus_ptr);
                // }
                int newIndex = *bgIndex_ptr + 1;
                bgIndex_ptr = &newIndex;
                while (wait3(childExitStatus_ptr, WNOHANG, usage_ptr)>0) {
                    getrusage(RUSAGE_SELF,&usage);
                    gettimeofday(&end, NULL);
                    printf("\n-- Statistics --\n");
                    printf("background\n");
                    printf("Elapsed time: %ld millisecond(s)\n", end.tv_sec * 1000 - start.tv_sec * 1000 + 
                            (end.tv_usec - start.tv_usec) / 1000);
                    faults[0] = usage.ru_majflt - faults[0];
                    faults[1] = usage.ru_minflt - faults[1];
                    printf("Page Faults: %ld\n", faults[0]);
                    printf("Page Faults (reclaimed): %ld \n", faults[1]);
                    printf("-- End of Statistics --\n\n");
                }
            }
            else{
                printf("waiting from foreground stuff\n");
                wait3Return = wait3(childExitStatus_ptr, 0, usage_ptr);
                while(wait3Return!=rc) { // pick up bg processes
                    if(wait3Return>0){
                        getrusage(RUSAGE_SELF,&usage);
                        gettimeofday(&end, NULL);
                        printf("\n-- Statistics --\n");
                        printf("not background\n");
                        printf("Elapsed time: %ld millisecond(s)\n", end.tv_sec * 1000 - start.tv_sec * 1000 + 
                            (end.tv_usec - start.tv_usec) / 1000);
                        faults[0] = usage.ru_majflt - faults[0];
                        faults[1] = usage.ru_minflt - faults[1];
                        printf("Page Faults: %ld\n", faults[0]);
                        printf("Page Faults (reclaimed): %ld \n", faults[1]);
                        printf("-- End of Statistics --\n\n");
                    }
                    wait3Return = wait3(childExitStatus_ptr, 0, usage_ptr);
                }

                getrusage(RUSAGE_SELF,&usage);
                gettimeofday(&end, NULL);
                printf("\n-- Statistics --\n");
                printf("not background\n");
                printf("Elapsed time: %ld millisecond(s)\n", end.tv_sec * 1000 - start.tv_sec * 1000 + 
                            (end.tv_usec - start.tv_usec) / 1000);
                faults[0] = usage.ru_majflt - faults[0];
                faults[1] = usage.ru_minflt - faults[1];
                printf("Page Faults: %ld\n", faults[0]);
                printf("Page Faults (reclaimed): %ld \n", faults[1]);
                printf("-- End of Statistics --\n\n");

                printf("time to wait for other tasks");
                if(end_of_file_flag){
                    wait3Return = wait3(childExitStatus_ptr, WNOHANG, usage_ptr);
                    while(wait3Return>=0) { 
                        if(wait3Return){
                            getrusage(RUSAGE_SELF,&usage);
                            gettimeofday(&end, NULL);
                            printf("\n-- Statistics --\n");
                            printf("not background\n");
                            printf("Elapsed time: %ld millisecond(s)\n", end.tv_sec * 1000 - start.tv_sec * 1000 + 
                                (end.tv_usec - start.tv_usec) / 1000);
                            faults[0] = usage.ru_majflt - faults[0];
                            faults[1] = usage.ru_minflt - faults[1];
                            printf("Page Faults: %ld\n", faults[0]);
                            printf("Page Faults (reclaimed): %ld \n", faults[1]);
                            printf("-- End of Statistics --\n\n");
                        }
                        wait3Return = wait3(childExitStatus_ptr, WNOHANG, usage_ptr);
                    }
                }
                printf("parent finished\n");
            }
            // while(wait(NULL)!=rc);
            // getrusage(RUSAGE_SELF,&usage);
            // gettimeofday(&end, NULL);
            // printf("\n-- Statistics --\n");
            // printf("Elapsed time: %ld millisecond(s)\n", (end.tv_usec - start.tv_usec) / 1000);
            // faults[0] = usage.ru_majflt - faults[0];
            // faults[1] = usage.ru_minflt - faults[1];
            // printf("Page Faults: %ld\n", faults[0]);
            // printf("Page Faults (reclaimed): %ld \n", faults[1]);
            // printf("-- End of Statistics --\n\n");
        }
    }
}

int main(int argc, char *argv[]) {
	char* file_path = "multi.txt";
    char* line;
    ssize_t size;
    size_t n = 0;

    // current directory variables
    char arr[LINE_MAX];
    char* currentDir = arr;
    char** currentDir_ptr = &currentDir;

    // background variables
    int i, bgArr[argc-1];
    int* bg = bgArr;
    int** bg_ptr = &bg;
    int bgIndex = 0;
    int* bgIndex_ptr = &bgIndex;
    int lineNum = 1;

    // parsing background line numbers from command line
    // for(i = 0; i < argc; i++)bg[i] = 0;
    for(i = 1; i < argc; i++) sscanf(argv[i],"%i",&bg[i-1]);
    // printf("line numbers of commands to run in the background\n");
    // for(i = 0; i < argc-1; i++) printf("%d ", bg[i]);
    // printf("\n");

    // get the current working directory
    getcwd(arr, sizeof(arr));

    // parsing
	FILE* file = fopen(file_path,"r"); // open the file
    size = getline(&line,&n,file); // get the next line from the file
    while(size >=0) {
        if(line[size-1]=='\n') line[size-1]='\0';
        execute(line, currentDir_ptr, lineNum, bgIndex_ptr, bg_ptr);
        lineNum++;
        size = getline(&line,&n,file);
    }
    end_of_file_flag = 1;

    return 0;
}


/*
 * Changes the current directory for this current process
 * as well as for any children processes. Called when
 * "ccd" is parsed.
 */
void changeDir(char** newDir_ptr) {
	if (chdir(*newDir_ptr) != 0) {
		perror(("Error changing the directory to %s\n", *newDir_ptr));

	}
}


/*
 * Prints out the name of the current working directory.
 */
void printDir(char** currentDir_ptr) {
	getcwd(*currentDir_ptr, sizeof(*currentDir_ptr));
	printf("Current directory: %s\n\n", *currentDir_ptr);
}

void cproclist(){
    printf("-- Background Processes --\n");
    int i;
    for(i = 0; i < bg_cnt;i++) printf("[%d] %s\n",i,bg_running[i]);
    printf("\n");
}