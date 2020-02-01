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


// proc_bg* bg_list = NULL;
bg_node* headNode = NULL;
int bgIndex = 0;
// int arr[100];
int* bg;
// int* bgIndex_ptr = &bgIndex;

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
 * Executes the given line and returns the updated linked list of processes
 * that are running in the background.
 */
void execute(char* command, char** currentDir_ptr, int lineNum) {
    struct rusage usage;
    struct timeval start, end;
    long int fg_faults[2] = {0,0};
    gettimeofday(&start,NULL);
    char* cmd_dup = strdup(command);

    // parent variables
    int childExitStatus;
    int* childExitStatus_ptr = &childExitStatus;
    pid_t wait3Return = 0;
    pid_t wait4Return = 0;
    pid_t processID = 0;
    pid_t* processID_ptr = &processID;


    printf("Running command: %s\n", command);
    char *myargs[34];
    myargs[33] = NULL;
    parse(command, myargs);
    if(strcmp(myargs[0],"ccd")==0){
        changeDir(&myargs[1]);
        *(currentDir_ptr) = myargs[1];
        printDir(currentDir_ptr);
    }
    else if(strcmp(myargs[0],"cpwd")==0){
        printDir(currentDir_ptr);
    }
    else if(strcmp(myargs[0],"cproclist")==0){
        printBgList();
    }
    else{
        changeDir(currentDir_ptr);
        int rc = fork();
        // *processID_ptr = getpid();
        // printf("fork process ID for command %s: %d\n", command, *processID_ptr);
        if (rc < 0) {
            // fork failed; exit
            fprintf(stderr, "fork failed\n");
            exit(1);
        }
        else if (rc == 0) { // CHILD
            printf("Child for %s\n", command);
            // *processID_ptr = getpid();
            // printf("child process ID for command %s: %d\n", command, *processID_ptr);
            getrusage(RUSAGE_SELF,&usage);
            fg_faults[0] = usage.ru_majflt;
            fg_faults[1] = usage.ru_minflt;
            if (bg[bgIndex] == lineNum) { // BACKGROUND - ADD TO LINKED LIST
                // addBgProc(fg_faults[0], fg_faults[1], &start, cmd_dup, *processID_ptr);
                // printBgList();
                proc_bg* new = findProc_Bg_cmd(cmd_dup);
                new->start_faults[0] = fg_faults[0];
                new->start_faults[1] = fg_faults[1];
                new->start_time = &start;
                new->pid = *processID_ptr;

                // proc_bg* tmp = findProc_Bg_pid(*processID_ptr);
                // if(tmp!=NULL)printf("added proc_bg with pid: %d\n", tmp->pid);
            }
            execvp(myargs[0], myargs);
            printf("Child for %s has exited\n", command);
        }
        else { // PARENT
            // printf("linenum: %d\n", lineNum);
            // printf("next bg ine num: %d\n", bg[bgIndex]);
            // *processID_ptr = rc;
            if (bg[bgIndex] == lineNum) { // BACKGROUND
                printf("Background parent for %s\n", command);
                // printf("child pid: %d\n", rc);
                // if (!isInList(*processID_ptr)) {
                //     addBgProc(fg_faults[0], fg_faults[1], &start, cmd_dup, *processID_ptr);
                //     printBgList();
                //     printf("Background child added to the linked list\n");
                // }
                // bg_running[bg_cnt] = strdup(command);
                // *bgIndex_ptr = *bgIndex_ptr + 1;
                // while(wait3(childExitStatus_ptr, WNOHANG, usage_ptr)){
                //     // printf("wait3 returned: %d\n", wait3Return);
                //     // printf("child exit status ptr: %d\n", *childExitStatus_ptr);
                // }
                bgIndex++;
                // printf("bg index: %d", )
                // bgIndex_ptr = &newIndex;
                while (wait3(childExitStatus_ptr, WNOHANG, &usage) > 0) {
                    // printf("about to print stats\n");
                    printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
                    // printf("finished printing stats\n");
                }
                // printf("wait3return is now %d\n", wait3Return);
                // while (wait3(childExitStatus_ptr, WNOHANG, &usage) == 0) {
                //     printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
                // }
                printf("Background parent for %s has exited\n", command);
            }
            else { // FOREGROUND
                // printf("is foreground for command %s, process ID %d\n", command, getpid());
                printf("Foreground parent for %s\n", command);
                // printf("child pid: %d\n", rc);
                while (wait4(*processID_ptr, childExitStatus_ptr, 0, &usage) <= 0) {
                    wait3Return = wait3(childExitStatus_ptr, WNOHANG, &usage);
                    // printf("wait 3 return %d, process id %d\n", wait3Return, *processID_ptr);
                    if (wait3Return > 0) {
                        if (wait3Return == *processID_ptr) {
                            printf("reached if\n");
                            return;
                        }
                        // printf("about to print stats\n");
                        printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
                        // printf("finished printing stats\n");
                    }
                    // printf("stuck in wait4\n");
                }
                // printf("foreground done\n");
                while (wait3(childExitStatus_ptr, WNOHANG, &usage) > 0) {
                    // printf("about to print stats\n");
                    printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
                    // printf("finished printing stats\n");
                }
                printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);
                printf("Foreground parent for %s has exited\n", command);
            }
            
            // getrusage(RUSAGE_SELF,&usage);
            // printStats(fg_faults[0], fg_faults[1], usage.ru_majflt, usage.ru_minflt,&start);

            // printf("lets check bg_list in parent\n");
            // printBgList();
        }
    }
}

int main(int argc, char *argv[]) {
    char* file_path = "multi.txt";
    char* line; 
    char* line_dup;
    ssize_t size;
    size_t n = 0;
    
    char arr[LINE_MAX];
    char* currentDir = arr;
    char** currentDir_ptr = &currentDir;

    // bg_list = (proc_bg*)malloc(sizeof(proc_bg));
    // bg_list->next = NULL;
    // bg_list->prev = NULL;

    // background variables
    int i, bgArr[argc-1];
    bg = bgArr;
    // int** bg_ptr = &bg;
    int lineNum = 1;
    int queue_num = 0;

    // get the current working directory
    getcwd(arr, sizeof(arr));

    // parsing background line numbers from command line
    for(i = 1; i < argc; i++) sscanf(argv[i],"%i",&bg[i-1]);

    // parsing
    FILE* file = fopen(file_path,"r");
    size = getline(&line,&n,file);
    line_dup = strdup(line);
    while(size >=0){
        if(line[size-1]=='\n') line[size-1]='\0';
        if (bg[bgIndex] == lineNum) { // BACKGROUND - ADD TO LINKED LIST
            addBgProc(0, 0, NULL, line_dup, -1);
            // printBgList();
        }
        execute(line, currentDir_ptr, lineNum);
        printf("\nprinting linked list in main after execute:\n");
        printBgList();
        printf("\n\n");
        lineNum++;
        size = getline(&line,&n,file);
    }

    int* childExitStatus_ptr;
    struct rusage usage;
    pid_t wait3Return = wait3(childExitStatus_ptr, WNOHANG, &usage);
    proc_bg* current = NULL;
    while (wait3Return >= 0) {
        // printf("straggling bg processes\n");
        if(wait3Return>0) printf("process with pid %d has finished\n", wait3Return);
        current = findProc_Bg_pid(wait3Return);
        if(current!=NULL){
            printf("found bg proc\n");
            printStats(current->start_faults[0], current->start_faults[1], usage.ru_majflt, usage.ru_minflt,current->start_time);
        }
        // printf("finished printing stats\n");
        wait3Return = wait3(childExitStatus_ptr, WNOHANG, &usage);
    }

    // testing bg_list
    // printf("initial bg_list\n");
    // printBgList();
    // printf("adding 3 bg tasks\n");
    // addBgProc(0,0,NULL,strdup("cmd1"),0);
    // addBgProc(0,0,NULL,strdup("cmd2"),1);
    // addBgProc(0,0,NULL,strdup("cmd3"),2);
    // printf("printing bg_list with 3 proc_bg\n");
    // printBgList();
    // printf("finding proc_bg with pid 1\n");
    // proc_bg* search = findProc_Bg(1);
    // if(search!=NULL) printf("found proc_bg with pid: %s\n", search->cmd);
    // printf("removing bg_list proc_bgs\n");
    // rmBgProc(0);
    // printBgList();
    // rmBgProc(2);
    // printBgList();
    // rmBgProc(1);
    // printBgList();


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

    if(bg_list == NULL){
        printf("bg_list points to NULL\n");
        return;
    }

    printf("-- Background Processes --\n");

    bg_node* current = headNode;
    // proc_bg* current = bg_list;
    while(current!=NULL){
        printf("[%d] %s\n", current->queue_num, current->command);
        current = current->next;
    }
}


void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd, pid_t pid){
    if(bg_list==NULL){
        bg_list = (proc_bg*)malloc(sizeof(proc_bg));
        bg_list->start_faults[0] = majflt;
        bg_list->start_faults[1] = minflt;
        bg_list->start_time = time;
        bg_list->cmd = cmd;
        bg_list->pid = pid;
        bg_list->queue_num = 0;
        bg_list->next = NULL;
        bg_list->prev = NULL;
        // return bg_list;
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
        new->pid = pid;
        new->queue_num = cnt;
        new->next = NULL;
        new->prev = current;
        printf("the new proc_bg's cmd: %s\n", new->cmd);
        current->next = new;
        printf("current->next's cmd: %s\n",current->next->cmd);
        // return new;
    }
    bg_node new = {command, queue_num, current, NULL};
    current->next = &new;
}

// void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd, pid_t pid){
//     if(bg_list==NULL){
//         bg_list = (proc_bg*)malloc(sizeof(proc_bg));
//         bg_list->start_faults[0] = majflt;
//         bg_list->start_faults[1] = minflt;
//         bg_list->start_time = time;
//         bg_list->cmd = cmd;
//         bg_list->pid = pid;
//         bg_list->queue_num = 0;
//         bg_list->next = NULL;
//         bg_list->prev = NULL;
//     }else{
//         proc_bg* current = bg_list;
//         int cnt = 1;
//         while(current->next!=NULL){
//             cnt++;
//             current = current->next;
//         }
//         printf("current is currently pointing to: %s\n", current->cmd);
//         proc_bg* new = (proc_bg*)malloc(sizeof(proc_bg));
//         new->start_faults[0] = majflt;
//         new->start_faults[1] = minflt;
//         new->start_time = time;
//         new->cmd = cmd;
//         new->pid = pid;
//         new->queue_num = cnt;
//         new->next = NULL;
//         new->prev = current;
//         printf("the new proc_bg's cmd: %s\n", new->cmd);
//         current->next = new;
//         printf("current->next's cmd: %s\n",current->next->cmd);
//     }
// }

void remBGNode(char* command) {
    bg_node* current = headNode;

    while (current != NULL) {
        if (strcmp(current->command, command) == 0) {
            bg_node* prev = current->prev;
            bg_node* next = current->next;
            if (prev == NULL) { // removing the first node
                headNode = current->next;
            }
            else if (next == NULL) { // removing the last node
                prev->next = NULL;
            }
            else {
                prev->next = next;
                next->prev = prev;
            }
            return;
        }
    }
}

proc_bg* findProc_Bg_pid(pid_t pid){
    if(bg_list==NULL) return NULL;
    proc_bg* current = bg_list;
    while(current!=NULL){
        if(current->pid == pid)return current;
        else current = current->next;
    }
    // printf("couldn't find proc_bg");
    return NULL;
}
proc_bg* findProc_Bg_cmd(char* cmd){
    if(bg_list==NULL) return NULL;
    proc_bg* current = bg_list;
    while(current!=NULL){
        if(strcmp(current->cmd, cmd)==0)return current;
        else current = current->next;
    }
    // printf("couldn't find proc_bg");
    return NULL;
}
