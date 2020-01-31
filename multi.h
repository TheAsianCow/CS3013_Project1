#ifndef HEADER_FILE
#define HEADER_FILE

int LINE_MAX = 128;

int main(int argc, char *argv[]);
void execute(char* command, char** currentDir_ptr, int lineNum, 
        int* bgIndex_ptr, int** bg_ptr);
void  parse(char *line, char **argv);
void changeDir(char** newDir_ptr);
void printDir(char** currentDir_ptr);
void printStats();
void printBgList();
void addBgProc(long int majflt, long int minflt, struct timeval* time, char* cmd);
#endif
