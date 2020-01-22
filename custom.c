#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <getopt.h>

int main(int argc, char *argv[]){
	char c;
	int help = 0;
	char* trace_path;
	while((c=getopt(argc,argv,"vhs:E:b:t:")) != -1){
        switch(c){
        case 'v':
            verb = 1;
            break;
        case 'h':
            help = 1;
            break;
        case 's':
            sets = atoi(optarg);
            if(sets==0){
            	printf("Invalid set size\n");
            	help = 1;
            }
            break;
        case 'E':
            ass = atoi(optarg);
            if(ass==0){
            	printf("Invalid associativity\n");
            	help = 1;
            }
            break;
        case 'b':
            block = atoi(optarg);
            if(block==0){
            	printf("Invalid block size\n");
            	help = 1;
            }
            break;
        case 't':
            trace_path = optarg;
            if(trace_path==NULL){
            	printf("Invalid path to trace file\n");
            	help = 1;
            }
            break;
        default:
            help = 1;
            printf("hit default\n");
            break;
        }
    }
    printf("retrieved args\n");
	if(help){
		printf("Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>\n");
		printf("-h: Optional help flag that prints usage info\n");
		printf("-v: Optional verbose flag that displays trace info \n");
		printf("-s <s>: Number of set index bits (the number of sets is 2s)\n");
		printf("-E <E>: Associativity (number of lines per set)\n");
		printf("-b <b>: Number of block bits (the block size is 2b)\n");
		printf("-t <tracefile>: Name of the valgrind trace to replay\n");
		exit(0);
	}
    return 0;
}