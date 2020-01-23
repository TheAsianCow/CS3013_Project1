#ifndef HEADER_FILE
#define HEADER_FILE

int LINE_MAX = 128;

int main(int argc, char *argv[]);
void execute(char* command);
void  parse(char *line, char **argv);
void changeDir(char* newDir);
void printDir(char* currentDir);

#endif
