README
CS 3013 Project 1
Jeffrey Huang and Jyalu Wu


----------------------------------------------------------------
Phase 1: boring.c
----------------------------------------------------------------
Functions:
	main
	execute
In order to execute the lines, we used a fork. The child process
uses a switch case to determine which command to run, then calls
execvp() on the command. The parent process prints out the stats
of the child process using the struct rusage, which can store and
get the number of page faults and the number of reclaimed page.
To get the time, we used the struct timeval and the gettimeofday()
function to compare the time we got at the start of the execute()
function to the time at the end of the execute() function.


----------------------------------------------------------------
Phase 2: custom.c
----------------------------------------------------------------
Functions:
	main
	parse
	execute
	changeDir
	printDir
In order to parse each line in the text file, we made a function
called parse() that stores each line in a char array. In order to
keep the directory change when ccd is called, we stored which
directory the program was supposed to be in at the moment in a
pointer that we passed around the program. Whenever a new line is
parsed, we immediately change the directory to the directory that
it is supposed to be at. The rest of the program is almost exactly
like what boring.c contains.


----------------------------------------------------------------
Phase 3: multi.c
----------------------------------------------------------------
Functions:			Structs:
	main				proc_bg
	execute
	parse
	changeDir
	printDir
	printStats
	printBgList
	addBgProc
	rmBgProc
	findProc_Bg_pid
	findProc_Bg_cmd
This file built on the existing custom.c file but allows processes
to run in the background. To do this, after forking, the child will
call execvp() if it is not a custom prompt that we wrote ourselves
and the parent will wait for processes to finish - how it does this
depends on if its corresponding child process is running in the
foreground or in the background. If it is running in the
background, the parent will check if any background processes
(except itself) have finished without blocking the program using
wait3() and WNOHANG. If it is running in the foreground, the
parent will wait for its corresponding child to finish using
wait4(). While it is waiting, it checks if any background processes
have finished and print out the statistics for those processes.
Once the foreground child has finished, the foreground parent will
print out the statistics for that child. However, since printing
the statistics takes time, it checks again to see if any other
background processes have finished and prints out the statistics 
for those.

In order to keep track of the processes that are currently running
in the background so we can print them out when cproclist is
specified in the text file, we made a linked list of structs that
represent the background processes. We add a proc_bg struct to
the list whenever a new process has to run in the background and
remove the struct from the list when it is done executing.


