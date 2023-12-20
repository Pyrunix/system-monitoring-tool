#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>
#include <signal.h>

#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <time.h>
#include <unistd.h>

void displayList(char *list[], int size)
{
    // Prints out all the strings in array list with size elements. If any elements in list are NULL, prints \n to move to next line
    for (int i = 0; i < size; i++)
    {
        if (list[i] != NULL)
        {
            printf("%s\n", list[i]);
        }
        else
        {
            printf("\n");
        }
    }
}

void displaySpecificItem(char *list[], int size, int itemIndex)
{
    // Prints out the string in array list of size elements at index itemIndex. Otherwise, print \n to move to next line
    for (int i = 0; i < size; i++)
    {
        if (list[i] != NULL && i == itemIndex)
        {
            printf("%s\n", list[i]);
        }
        else
        {
            printf("\n");
        }
    }
}

void handle_sigtstp(int sig)
{
    // An empty function to replace the SIGTSTP handler (effectively ignoring it)
}

void handle_sigint(int sig)
{   
    // A signal handler to replace SIGINT with a prompt to exit
    printf("\n Would you like to quit the program? [Enter 1 to quit]: ");
    int input = 0;
    scanf("%d", &input);
    if (input == 1)
    {
        exit(0);
    }
}

int isNumber(char *str)
{
    // Function takes in a string str and returns 1 if str contains only numbers, 0 otherwise
    int isNum = 1;
    for (int i = 0; i < strlen(str); i++)
    {
        if (!isdigit(str[i]))
        {
            isNum = 0;
        }
    }
    return isNum;
}

int main(int argc, char **argv)
{
    int samples = 10;
    int delay = 1;
    bool showSystem = 0;
    bool showUser = 0;
    bool graphics = 0;
    bool sequential = 0;
    int positionCheck = 0;

    // Check if there are any arguments
    if (argc > 1)
    {

        // Assign variables according to arguments given
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--system") == 0)
            {
                showSystem = true;
            }
            else if (strcmp(argv[i], "--user") == 0)
            {
                showUser = true;
            }
            else if (strcmp(argv[i], "--graphics") == 0)
            {
                graphics = true;
            }
            else if (strcmp(argv[i], "--sequential") == 0)
            {
                sequential = true;
            }
            else if (strstr(argv[i], "--samples=") != NULL)
            {
                samples = 0;
                positionCheck = 1;
                for (int j = 0; j < strlen(argv[i]); j++)
                {
                    if (isdigit(argv[i][j]))
                    {
                        int x = (argv[i][j]) - '0';
                        samples = samples * 10 + x;
                    }
                }
            }
            else if (strstr(argv[i], "--tdelay=") != NULL)
            {
                delay = 0;
                for (int j = 0; j < strlen(argv[i]); j++)
                {
                    if (isdigit(argv[i][j]))
                    {
                        int x = (argv[i][j]) - '0';
                        delay = delay * 10 + x;
                    }
                }
                positionCheck = 0;
            }
            else if (isNumber(argv[i]))
            {
                if (positionCheck == 0)
                {
                    // If this is the first argument without a flag or tdelay has already been set, set sample
                    samples = 0;
                    positionCheck = 1;
                    for (int j = 0; j < strlen(argv[i]); j++)
                    {
                        if (isdigit(argv[i][j]))
                        {
                            int x = (argv[i][j]) - '0';
                            samples = samples * 10 + x;
                        }
                    }
                }
                else if (positionCheck == 1)
                {
                    positionCheck = 0;
                    delay = 0;
                    for (int j = 0; j < strlen(argv[i]); j++)
                    {
                        if (isdigit(argv[i][j]))
                        {
                            int x = (argv[i][j]) - '0';
                            delay = delay * 10 + x;
                        }
                    }
                }
            }
        }
    }

    struct sigaction sa;
    sa.sa_handler = &handle_sigtstp;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa, NULL);
    struct sigaction sa_int;
    sa_int.sa_handler = &handle_sigint;
    sigaction(SIGINT, &sa_int, NULL);

    // Show both system and user info by default
    if (showUser == 0 && showSystem == 0)
    {
        showUser = 1;
        showSystem = 1;
    }

    // Initializing variables
    char *memOutputs[samples];
    for (int i = 0; i < samples; i++)
    {
        memOutputs[i] = calloc(528, sizeof(char));
    }
    char *cpuOutputs[samples];
    for (int i = 0; i < samples; i++)
    {
        cpuOutputs[i] = calloc(528, sizeof(char));
    }
    long double lastRam = 0;
    long int lastTotalUsage, lastIdleUsage;
    lastTotalUsage = 0;

    clock_t start = clock();
    for (int i = 0; i < samples;)
    {
        if (((double)(clock() - start)) / CLOCKS_PER_SEC >= delay)
        {
            if (sequential == 0)
            {
                // If sequential is set to false, refresh the screen
                system("clear");
            }

            printf("Showing %d samples at %d second intervals\n", samples, delay);

            // Memory Usage
            int fd[2];
            if (pipe(fd) == -1)
            {
                fprintf(stderr, "Error opening the pipe");
                exit(1);
            }
            int id = fork();
            if (id == -1)
            {
                fprintf(stderr, "Error forking");
                exit(1);
            }
            if (id == 0)
            {
                if (dup2(fd[1], STDOUT_FILENO) == -1)
                {
                    fprintf(stderr, "Error duplicating pipe file descriptor");
                    exit(1);
                }
                if (close(fd[0]) == -1)
                {
                    fprintf(stderr, "Error closing pipe");
                    exit(1);
                }
                if (close(fd[1]) == -1)
                {
                    fprintf(stderr, "Error closing pipe");
                    exit(1);
                }
                execlp("./stats", "stats", "--intro", (char *)NULL);
            }
            else
            {
                if (close(fd[1]) == -1)
                {
                    fprintf(stderr, "Error closing pipe");
                    exit(1);
                }
                char *output = calloc(528, sizeof(char));
                read(fd[0], output, 528);
                if (close(fd[0]) == -1)
                {
                    fprintf(stderr, "Error closing pipe");
                    exit(1);
                }
                printf("%s", output);
                free(output);
            }

            // Memory Utilization
            if (showSystem)
            {
                int fd[2];
                if (pipe(fd) == -1)
                {
                    fprintf(stderr, "Error opening the pipe");
                    exit(1);
                }
                int id = fork();
                if (id == -1)
                {
                    fprintf(stderr, "Error forking");
                    exit(1);
                }
                if (id == 0)
                {
                    if (dup2(fd[1], STDOUT_FILENO) == -1)
                    {
                        fprintf(stderr, "Error duplicating pipe file descriptor");
                        exit(1);
                    }
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *tmp = calloc(100, sizeof(char));
                    snprintf(tmp, 100, "--lastRam=%d", (int)(lastRam * 10000));
                    if (graphics)
                    {
                        execlp("./stats", "stats", "--mem", "--graphics", tmp, (char *)NULL);
                    }
                    else
                    {
                        execlp("./stats", "stats", "--mem", tmp, (char *)NULL);
                    }
                    fprintf(stderr, "Error executing stats.exe");
                    exit(1);
                }
                else
                {
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *output = calloc(528, sizeof(char));
                    read(fd[0], output, 528);
                    // printf("\n%s\n", output);
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *tmp = calloc(528, sizeof(char));
                    char *token = strtok(output, ":");
                    strncpy(tmp, token, 528);
                    lastRam = atof(tmp);

                    token = strtok(NULL, ":");
                    strncpy(memOutputs[i], token, 528);
                    free(output);
                    free(tmp);

                    // Show data
                    printf("\n--- Memory --- (Physical Used / Total -- Virtual Used / Total)\n");
                    if (sequential == 0)
                    {
                        // If sequential is set to false, display all memory info generated
                        displayList(memOutputs, samples);
                    }
                    else
                    {
                        // If sequential is set to true, only print the information generated at the current sample
                        displaySpecificItem(memOutputs, samples, i);
                    }
                }
            }

            // User information
            if (showUser)
            {
                int fd[2];
                // fd[0] is read
                // fd[1] is write
                if (pipe(fd) == -1)
                {
                    fprintf(stderr, "Error opening the pipe");
                    exit(1);
                }
                int id = fork();
                if (id == 0)
                {
                    if (dup2(fd[1], STDOUT_FILENO) == -1)
                    {
                        fprintf(stderr, "Error duplicating pipe file descriptor");
                        exit(1);
                    }
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    execlp("./stats", "stats", "--user", (char *)NULL);
                    fprintf(stderr, "Error executing stats.exe");
                    exit(1);
                }
                else
                {
                    printf("\n--- Sessions / Users ---\n");
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *output = calloc(2048, sizeof(char));
                    read(fd[0], output, 2048);
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    printf("%s", output);
                    free(output);
                }
            }

            // CPU Usage Graphics
            if (showSystem)
            {
                int fd[2];
                if (pipe(fd) == -1)
                {
                    fprintf(stderr, "Error opening the pipe");
                    exit(1);
                }
                int id = fork();
                if (id == -1)
                {
                    fprintf(stderr, "Error forking");
                    exit(1);
                }
                if (id == 0)
                {
                    if (dup2(fd[1], STDOUT_FILENO) == -1)
                    {
                        fprintf(stderr, "Error duplicating pipe file descriptor");
                        exit(1);
                    }
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *input1 = calloc(100, sizeof(char));
                    char *input2 = calloc(100, sizeof(char));
                    snprintf(input1, 100, "%ld", lastTotalUsage);
                    snprintf(input2, 100, "%ld", lastIdleUsage);
                    execlp("./stats", "stats", "--cpu", "--graphics", input1, input2, (char *)NULL);
                    fprintf(stderr, "Error executing stats");
                    exit(1);
                }
                else
                {
                    if (close(fd[1]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *output = calloc(528, sizeof(char));
                    read(fd[0], output, 528);
                    if (close(fd[0]) == -1)
                    {
                        fprintf(stderr, "Error closing pipe");
                        exit(1);
                    }
                    char *token = NULL;
                    token = strtok(output, ";");
                    printf("%s\n", token);
                    token = strtok(NULL, ";");
                    lastTotalUsage = atol(token);
                    token = strtok(NULL, ";");
                    lastIdleUsage = atol(token);
                    token = strtok(NULL, ";");
                    printf("%s", token);
                    token = strtok(NULL, ";");
                    strncpy(cpuOutputs[i], token, 528);
                    token = strtok(NULL, ";");

                    if (graphics == 1)
                    {
                        // Show data
                        if (sequential == 0)
                        {
                            // If sequential is set to false, display all memory info generated
                            displayList(cpuOutputs, samples);
                        }
                        else
                        {
                            // If sequential is set to true, only print the information generated at the current sample
                            displaySpecificItem(cpuOutputs, samples, i);
                        }
                    }

                    printf("\n--- System Information ---\n");

                    printf("%s", token);
                    free(output);
                }
            
            }
            fflush(stdout);
            start = clock();
            i++;
        }
    }
    return 0;
}