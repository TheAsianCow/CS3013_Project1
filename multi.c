#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <getopt.h>
#include <pthread.h>
#include "multi.h"
// #include <libexplain/gcc_attributes.h>

typedef struct proc_bg{
    int done = 0;
    long int start_faults[2];
    struct timeval* start_time;
    char* cmd;
    int queue_num;
    struct proc_bg* next;
    struct proc_bg* prev;
}proc_bg;

typedef struct execArgs {
    int isBG;
    char* command;
    char** currentDir_ptr;
} execArgs;


proc_bg* bg_list = NULL;

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
void* execute(void* arguments){
    struct execArgs *args = arguments;
    struct rusage usage;
    struct timeval start;
    long int fg_faults[2] = {0,0};
    gettimeofday(&start,NULL);


    printf("Running command: %s\n",args->command);
    char *myargs[34];
    myargs[33] = NULL;
    parse(args->command, myargs);
    if(strcmp(myargs[0],"ccd")==0){
        changeDir(&myargs[1]);
        *(args->currentDir_ptr) = myargs[1];
        printDir(args->currentDir_ptr);
    }
    else if(strcmp(myargs[0],"cpwd")==0){
        printDir(args->currentDir_ptr);
    }
    else if(strcmp(myargs[0],"cproclist")==0){
        printBgList();
    }
    else{
        changeDir(args->currentDir_ptr);
        int rc = fork();
        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0) { // CHILD
            getrusage(RUSAGE_SELF,&usage);
            fg_faults[0] = usage.ru_majflt;
            fg_faults[1] = usage.ru_minflt;
            if (args->isBG == 1) {
                addBgProc(fg_faults[0], fg_faults[1], &start, args->command);
            }
            execvp(myargs[0], myargs);
            if (args->isBG) {
                updateBGThreadStatus();
            }
        }
        else { // PARENT
            while(wait(NULL)!=rc);
            getrusage(RUSAGE_SELF,&usage);
            printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
        }
    }
}

int main(int argc, char *argv[]) {
    char* file_path = "multi.txt";
    char* line; 
    ssize_t size;
    size_t n = 0;
    
    char arr[LINE_MAX];
    char* currentDir = arr;
    char** currentDir_ptr = &currentDir;

    // background variables
    int i, bgArr[argc-1];
    int* bg = bgArr;
    int bgIndex = 0;
    int lineNum = 1;

    // thread variables
    pthread_t bgThread;
    int bgThreadRet;
    struct execArgs executeArgs = {0, line, currentDir_ptr};

    // get the current working directory
    getcwd(arr, sizeof(arr));

    // parsing background line numbers from command line
    for(i = 1; i < argc; i++) sscanf(argv[i],"%i",&bg[i-1]);

    // parsing
    FILE* file = fopen(file_path,"r");
    size = getline(&line,&n,file);
    while(size >=0){
        executeArgs.currentDir_ptr = currentDir_ptr;
        executeArgs.command = line;
        if(line[size-1]=='\n') line[size-1]='\0';
        if (bg[bgIndex] == lineNum) { // BACKGROUND
            bgThreadRet = pthread_create(&bgThread, NULL, &execute, (void*) &executeArgs);
        }
        else {
            execute(&executeArgs);
        }
        lineNum++;
        size = getline(&line,&n,file);
        if (bgThreadRet == 0) {
            pthread_join(bgThread, NULL);
        }
    }

    //testing bg_list
    printf("initial bg_list\n");
    printBgList();
    printf("adding 3 bg tasks\n");
    addBgProc(0,0,NULL,strdup("cmd1"));
    addBgProc(0,0,NULL,strdup("cmd2"));
    addBgProc(0,0,NULL,strdup("cmd3"));
    printf("printing bg_list with 3 proc_bg\n");
    printBgList();

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

void printStats(long int start_majflt, long int start_minflt, long int end_majflt, long int end_minflt, struct timeval* start){
    struct timeval end;
    gettimeofday(&end, NULL);

    printf("\n-- Statistics --\n");
    printf("Elapsed time: %ld millisecond(s)\n", (end.tv_sec - start->tv_sec)*1000 + (end.tv_usec - start->tv_usec) / 1000);
    printf("Page Faults: %ld\n", end_majflt-start_majflt);
    printf("Page Faults (reclaimed): %ld \n", end_minflt-start_minflt);
    printf("-- End of Statistics --\n\n");
}

void printBgList(){
    printf("-- Background Processes --\n");

    proc_bg* current = bg_list;
    if(bg_list!=NULL){
        while(current!=NULL){
            printf("[%d] %s\n", current->queue_num, current->cmd);
            current = current->next;
        }
    }
    printf("\n");
}

void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd){
    if(bg_list==NULL){
        bg_list = (proc_bg*)malloc(sizeof(proc_bg));
        bg_list->start_faults[0] = majflt;
        bg_list->start_faults[1] = minflt;
        bg_list->start_time = time;
        bg_list->cmd = cmd;
        bg_list->queue_num = 0;
        bg_list->next = NULL;
    }else{
        proc_bg* current = bg_list;
        int cnt = 1;
        while(current->next!=NULL){
            cnt++;
            current = current->next;
        }
        printf("current is currently pointing to: %s\n", current->cmd);
        proc_bg* new = (proc_bg*)malloc(sizeof(proc_bg));
        new->start_faults[0] = majflt;
        new->start_faults[1] = minflt;
        new->start_time = time;
        new->cmd = cmd;
        new->queue_num = cnt;
        new->next = NULL;
        printf("the new proc_bg's cmd: %s\n", new->cmd);
        current->next = new;
        printf("current->next's cmd: %s\n",current->next->cmd);
    }
}

void rmBgProc(){

}

updateBGThreadStatus() {
    
}