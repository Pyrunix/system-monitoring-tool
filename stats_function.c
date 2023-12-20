#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

#include <sys/resource.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <time.h>
#include <unistd.h>

void displayMemoryUsage()
{
    // Prints memory used by current application in kB using information taken from /proc/self/status
    FILE *file = fopen("/proc/self/status", "r");
    int memUsage = -1;
    char line[128];

    while (fgets(line, 128, file) != NULL)
    {
        if (strncmp(line, "VmSize:", 7) == 0)
        {
            char *ptr = line;
            while (!isdigit(*ptr) && *ptr != '\0')
            {
                ptr++;
            }
            memUsage = atoi(ptr);
            break;
        }
    }
    fclose(file);
    printf("Memory Usage: %d kB\n", memUsage);
}

void displayUsers()
{
    // Prints all user, TTY, and host information taken from /var/run/utmp for USER_PROCESS types
    struct utmp *userInfo = NULL;
    userInfo = getutent();

    while (userInfo != NULL)
    {
        if (userInfo->ut_type == USER_PROCESS)
        {
            printf("%s\r\t\t | TTY: %s\t | HOST: %s\n", userInfo->ut_user, userInfo->ut_line, userInfo->ut_host);
        }
        userInfo = getutent();
    }
    setutent();
}

void displaySystemInfo()
{
    // Display system information such as operating system name, release, version, and machine using information in sys/utsname.h
    struct utsname systemInfo;
    uname(&systemInfo);
    printf("System Name: %s \nMachine Name: %s\nVersion: %s \nRelease: %s \nArchitecture: %s \n",
           systemInfo.sysname, systemInfo.nodename, systemInfo.version, systemInfo.release, systemInfo.machine);
}

void generateMemoryInfo(bool graphics, long double *lastRam, char *outputString)
{
    // Generates a string stating used and free memory in system, if graphics is true then shows a graphical display and compares ram usage with lastRam.
    // outputString is set to the generated string, at a maximum of 528 characters including NULL terminating character
    struct sysinfo memoryInfo;
    sysinfo(&memoryInfo);
    long double totalPhysicalMem = memoryInfo.totalram;
    long double totalVirtualMem = totalPhysicalMem + memoryInfo.totalswap;
    long double usedPhysicalMem = totalPhysicalMem - memoryInfo.freeram;
    long double usedVirtualMem = totalVirtualMem - memoryInfo.freeram;
    // Resize memory to show in GB
    totalPhysicalMem = totalPhysicalMem * memoryInfo.mem_unit / (1000000 * 1024);
    totalVirtualMem = totalVirtualMem * memoryInfo.mem_unit / (1000000 * 1024);
    usedPhysicalMem = usedPhysicalMem * memoryInfo.mem_unit / (1000000 * 1024);
    usedVirtualMem = usedVirtualMem * memoryInfo.mem_unit / (1000000 * 1024);

    if (!graphics)
    {
        printf("%Lf:", usedPhysicalMem);
        snprintf(outputString, 527, "%0.2Lf GB / %0.2Lf GB -- %0.2Lf GB / %0.2Lf GB", usedPhysicalMem, totalPhysicalMem, usedVirtualMem, totalVirtualMem);
    }
    else
    {
        printf("%Lf:", usedPhysicalMem);
        snprintf(outputString, 527, "%0.2Lf GB / %0.2Lf GB -- %0.2Lf GB / %0.2Lf GB   |  ", usedPhysicalMem, totalPhysicalMem, usedVirtualMem, totalVirtualMem);
        if (*lastRam != 0)
        {
            // Check if this is the first call, if not then compare current RAM usage to previous RAM usage in lastRam
            if (*lastRam <= usedPhysicalMem)
            {
                // If used physical memory increased since last check, add in positive graphics
                int count = 0;
                for (int i = 1; i < (usedPhysicalMem - *lastRam) * 100; i++)
                {
                    strcat(outputString, "#");
                    count++;
                }
                snprintf(outputString + strlen(outputString), 527 - count - strlen(outputString), "* %0.2Lf (%0.2Lf)", usedPhysicalMem - *lastRam, usedPhysicalMem);
            }
            else if (*lastRam > usedPhysicalMem)
            {
                // If used physical memory decreased since last check, add in positive graphics
                int count = 0;
                for (int i = 1; i < (usedPhysicalMem - *lastRam) * 100; i++)
                {
                    strcat(outputString, ":");
                    count++;
                }
                snprintf(outputString + strlen(outputString), 527 - count - strlen(outputString), "@ %0.2Lf (%0.2Lf)", usedPhysicalMem - *lastRam, usedPhysicalMem);
            }
        }
        *lastRam = usedPhysicalMem;
    }
}

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

void generateCpuUsage(bool graphics, long int *lastIdleUsage, long int *lastTotalUsage, char *outputString)
{
    // Prints information about CPU regarding total CPU usage by comparing current CPU usage to lastIdleUsage and lastTotalUsage.
    // If graphics is set to true, then a string of max 528 characters (including NULL terminating character) will be generated as graphics, and put into string outputString
    FILE *file = fopen("/proc/stat", "r");
    long int userUsage, niceUsage, systemUsage, idleUsage, ioUsage, irqUsage, softirqUsage, totalUsage;
    // Only concerned with first 7 fields, as 10 fields are only provided as of Linux kernel 2.6.33
    fscanf(file, "cpu %ld %ld %ld %ld %ld %ld %ld",
           &userUsage, &niceUsage, &systemUsage, &idleUsage, &ioUsage, &irqUsage, &softirqUsage);

    totalUsage = userUsage + niceUsage + systemUsage + ioUsage + irqUsage + softirqUsage;

    if (*lastTotalUsage <= 0)
    {
        *lastTotalUsage = totalUsage;
        *lastIdleUsage = idleUsage;
        if (graphics == 1)
        {
            strcat(outputString, "\t\t| 0.00"); // At first call, CPU usage is 0 as CPU usage is calculated over time
        }
        printf("%ld;%ld;", totalUsage, idleUsage);
        printf("Total CPU Usage: 0.00%% ; \n");
        return;
    }

    long int deltaTotalUsage = abs(*lastTotalUsage - totalUsage);
    long int deltaIdleUsage = abs(*lastIdleUsage - idleUsage);
    long double percentUsage = deltaTotalUsage;
    percentUsage /= (deltaIdleUsage + deltaTotalUsage);
    percentUsage *= 100;
    printf("%ld;%ld;", totalUsage, idleUsage);
    printf("Total CPU Usage: %0.2Lf%% ;", percentUsage);
    if (graphics == 1)
    {
        printf("\t\t");
        for (int i = 0; i < percentUsage; i++)
        {
            printf("|");
        }
        printf(" %0.2Lf", percentUsage);
    }
}

void displayCpuCores()
{
    // Reads information from /proc/cpuinfo and prints out the number of cores the computer has
    int cores = -1;
    FILE *file = fopen("/proc/cpuinfo", "r");
    if (file == NULL)
    {
        printf("Error getting CPU Info\n");
        return;
    }
    char line[128];

    while (fgets(line, 128, file) != NULL)
    {

        if (strncmp(line, "cpu cores", 9) == 0)
        {
            char *ptr = line;
            while (!isdigit(*ptr) && *ptr != '\0')
            {
                ptr++;
            }
            cores = atoi(ptr);
            break;
        }
    }

    printf("Number of Cores: %d;", cores);
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
    bool showUser = 0;
    bool graphics = 0;
    int positionCheck = 0;
    bool intro = 0;
    bool showMem = 0;
    bool showCpu = 0;

    long double lastRam = 0;
    long int lastTotalUsage, lastIdleUsage;
    lastTotalUsage = 0;

    // Check if there are any arguments
    if (argc > 1)
    {

        // Assign variables according to arguments given
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "--user") == 0)
            {
                showUser = true;
            }
            else if (strcmp(argv[i], "--graphics") == 0)
            {
                graphics = true;
            }
            else if (strcmp(argv[i], "--intro") == 0)
            {
                intro = true;
            }
            else if (strcmp(argv[i], "--mem") == 0)
            {
                showMem = true;
            }
            else if (strcmp(argv[i], "--cpu") == 0)
            {
                showCpu = true;
            }
            else if (strstr(argv[i], "--lastRam=") != NULL)
            {
                lastRam = 0;
                positionCheck = 1;
                for (int j = 0; j < strlen(argv[i]); j++)
                {
                    if (isdigit(argv[i][j]))
                    {
                        int x = (argv[i][j]) - '0';
                        lastRam = lastRam * 10 + x;
                    }
                }
                lastRam /= 10000;
            }
            else if (isNumber(argv[i]))
            {
                if (positionCheck == 0)
                {
                    // If this is the first argument without a flag, set lastTotalUsage
                    lastTotalUsage = 0;
                    positionCheck = 1;
                    for (int j = 0; j < strlen(argv[i]); j++)
                    {
                        if (isdigit(argv[i][j]))
                        {
                            int x = (argv[i][j]) - '0';
                            lastTotalUsage = lastTotalUsage * 10 + x;
                        }
                    }
                }
                else if (positionCheck == 1)
                {
                    positionCheck = 0;
                    lastIdleUsage = 0;
                    for (int j = 0; j < strlen(argv[i]); j++)
                    {
                        if (isdigit(argv[i][j]))
                        {
                            int x = (argv[i][j]) - '0';
                            lastIdleUsage = lastIdleUsage * 10 + x;
                        }
                    }
                }
            }
        }
    }

    // Initializing variables
    char* output;

    // Show memory usage
    if (intro){
        displayMemoryUsage();
    }
    // Show memory utilization
    if (showMem){
        output = malloc(sizeof(char) * 528);
        generateMemoryInfo(graphics, &lastRam, output);
        printf("%s", output);
        free(output);
    }
    // Show user info
    if (showUser)
    {
        displayUsers();
    }

    // Show CPU usage
    if (showCpu)
    {   
        displayCpuCores();
        output = malloc(sizeof(char) * 528);
        generateCpuUsage(graphics, &lastIdleUsage, &lastTotalUsage, output);
        printf(";");
        displaySystemInfo();
        free(output);
    }

    return 0;
}
