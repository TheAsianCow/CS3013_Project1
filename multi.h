#ifndef HEADER_FILE
#define HEADER_FILE

#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>

int LINE_MAX = 128;

int main(int argc, char *argv[]);
void execute(char* command, char** currentDir_ptr, int lineNum);
void  parse(char *line, char **argv);
void changeDir(char** newDir_ptr);
void printDir(char** currentDir_ptr);
void printStats();
void printBgList();
void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd, pid_t pid);
void rmBgProc(pid_t pid);
#endif
