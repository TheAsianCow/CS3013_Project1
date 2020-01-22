all: boring custom

boring: boring.o
	gcc -o boring boring.o

boring.o: boring.c
	gcc -c boring.c

custom: custom.o
	gcc -o custom custom.o

custom.o: boring.c
	gcc -c custom.c

clean:
	rm boring.o boring
	rm custom.o custom
