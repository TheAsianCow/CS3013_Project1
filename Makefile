CC=gcc

all: boring custom

boring: boring.o
	$(CC) -o $@ boring.o

boring.o: boring.c
	$(CC) -c boring.c

custom: custom.o
	$(CC) -o $@ custom.o

custom.o: boring.c
	$(CC) -c custom.c

clean:
	rm boring.o boring
	rm custom.o custom
