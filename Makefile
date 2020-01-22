all: boring

boring: boring.o
	gcc -o boring boring.o

boring.o: boring.c
	gcc -c boring.c

clean:
	rm boring.o boring
