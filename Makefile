CC=gcc

all: boring custom multi

boring: boring.o
	$(CC) -o $@ $^

boring.o: boring.c
	$(CC) -c $^

custom: custom.o
	$(CC) -o $@ $^

custom.o: custom.c custom.h
	$(CC) -c $^

multi: multi.o
	$(CC) -o $@ $^

multi.o: multi.c multi.h
	$(CC) -c $^

clean:
	rm boring.o boring
	rm custom.o custom
