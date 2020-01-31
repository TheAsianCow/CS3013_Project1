#ifndef HEADER_FILE
#define HEADER_FILE

#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

int LINE_MAX = 128;

typedef struct proc_bg{
    int done;
    long int start_faults[2];
    struct timeval* start_time;
    char* cmd;
    pid_t pid;
    int queue_num;
    struct proc_bg* next;
    struct proc_bg* prev;
}proc_bg;

int main(int argc, char *argv[]);
void execute(char* command, char** currentDir_ptr, int lineNum);
void  parse(char *line, char **argv);
void changeDir(char** newDir_ptr);
void printDir(char** currentDir_ptr);
void printStats(long int start_majflt, long int start_minflt, long int end_majflt, long int end_minflt, struct timeval* start);
void printBgList();
void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd, pid_t pid);
void rmBgProc(pid_t pid);
proc_bg* findProc_Bg_pid(pid_t pid);
proc_bg* findProc_Bg_cmd(char* cmd);

#endif
