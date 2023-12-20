# mySystemStats.c Documentation

### By Darren Trieu

## How to use 
After compiling the a3.c file, it can be run with the command line arguments in any order. The stats_function.c must be in the
same directory, and be compiled into "stats" (via "gcc stats_function.c -o stats"). This can be done via "make all", as a
Makefile is provided. By default, graphics is set to off,
samples is set to 10, delay is set to 1, sequential is set to false, and will generate both system and user usage. Only
integer numbers will be accepted to set samples and tdelay.

--system
	-will only generate system usage (if used alongside --user, will show both user and system usage)
--user
	-will only generate user usage (if used alongside --system, will show both user and system usage)
--graphics
	-will generate graphical outputs for memory usage and CPU usage
--sequential
	-output will be done sequentially, without refreshing the screen
--samples=N
	-sets the sample to N, where the program will check system/user usage N times
--tdelay=T
	-sets the delay to T, where the program will check system/user usage at T second intervals

Note that samples and tdelay can also be used as positional arguments if flag is not indicated in the order (samples) (tdelay).
For example, if the output is named "a3" we can run the program with
./a3 --graphics --user 2 4             (Graphics on, show user data only, take 2 samples at 4 second intervals)
./a3 6 --system                        (Show system data only, take 6 samples at 1 (default) second intervals)
./a3 --tdelay=5 8                      (Take 8 samples at 5 second delays)
./a3 4 --sequential 2                  (Take 4 samples at 2 second delays, print information sequentially)

## Function Overviews
The functions in stats_function.c remain relatively the same as mySystemStats.c from A1, most of the changes were made to the main().

void displayList(char *list[], int size)
displayList prints out all the strings in array list with size elements. 
If any elements in list are NULL, print \n to move to next line

void displaySpecificItem()
displaySpecificItem prints out the string in array list of size elements at index itemIndex. Otherwise, prints \n to move on to next line

int isNumber(char *str)
isNumber checks if the string str contains only numbers, returns 1 if yes and 0 otherwise.

void handle_sigtstp(int sig)
An empty function handler that is used to replace the CTRL-Z signal handler, as the function should not be run in the background

void handle_sigint(int sig)
A function that is used to replace the signal handler for CTRL-C, which instead asks if the user would like to quit. If '1' is entered, the program exits.
Otherwise, the function continues.