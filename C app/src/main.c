#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include "main.h"

#define FILENAME "main" 
#define TRAIN_FATHER_NAME "train_father"
#define TRAIN_PROCESS_NAME "train_process"
#define RBC_NAME "rbc"


int main(int argc, char **argv)
{
    char mode[6];
    char map[5];

    // Argument handling with assertions. Assigns correct values to mode and map strings.
    // 'mode' can be either ETCS1, ETCS2, or RBC. 'map' can be either MAP1 or MAP2.
    check_arguments(argc, argv, mode, map);

    printf("Launching processes. [MODE: %s] [MAP: %s].\nCheck log files for more information.\n ", mode, map);

    char train_father_path[300];
    char train_process_path[300];
    char rbc_path[300];

    // Builds the paths for train_father_path, train_process_path, and rbc_path, which are all absolute paths to the corresponding executables.
    create_abspaths(argv[0], train_father_path, train_process_path, rbc_path);

    // Create processes:
    // if mode = RBC: create 'rbc' process
    // if mode = ETCS1 or mode = ETCS2: create 'train_father" process
    create_child_processes(mode, map, train_father_path, train_process_path, rbc_path);

    return 0;
}


void check_arguments(int argc, char* argv[], char* mode, char* map)
{
    assert(argc > 2 && argc < 5);
    if (argc >= 3)
    {
        assert(strncmp(argv[1], "ETCS1", 6) == 0 || strncmp(argv[1], "ETCS2", 6) == 0);
        strncpy(mode, argv[1], 6);

        if (argc == 3)
        {
            assert(strncmp(argv[2], "MAP1", 5) == 0 || strncmp(argv[2], "MAP2", 5) == 0);     
            strncpy(map, argv[2], 5);
        }
        if (argc == 4)
        {
            assert(strncmp(argv[2], "RBC", 4) == 0);
            assert(strncmp(argv[3], "MAP1", 5) == 0 || strncmp(argv[3], "MAP2", 5) == 0);
            strncpy(mode, argv[2], 4);
            strncpy(map, argv[3], 5);
        }
    }
}


void create_abspaths(char* argv0, char* train_father_path, char* train_process_path, char* rbc_path)
{
    char abspath[150];
    realpath(argv0, abspath);
    abspath[strlen(abspath) - strlen(FILENAME)] = '\0';
    // abspath now contains the absolute path for the root folder of the executables
    
    // We can then complete the path for every executable


    sprintf(train_father_path, "%s" TRAIN_FATHER_NAME, abspath);
    sprintf(train_process_path, "%s" TRAIN_PROCESS_NAME, abspath);
    sprintf(rbc_path, "%s" RBC_NAME, abspath);
}


void create_child_processes(char* mode, char* map, char* train_father_path, char* train_process_path, char* rbc_path)
{
    pid_t pid = fork();
    
    // Create TRAIN_FATHER process
    if (pid < 0)
    {
        printf("Error while forking\n");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0) /*child process*/
    {
        if (strncmp(mode, "RBC", 6) == 0)
        {
            // printf("Launching RBC server process with PID %d\n", getpid());
            execl(rbc_path, RBC_NAME, map, NULL);
        }

        else
        {
            // printf("Launching TRAIN_FATHER process with PID %d\n", getpid());
            execl(train_father_path, TRAIN_FATHER_NAME, mode, map, train_process_path, NULL);
        }
    }
    
    wait(NULL);
}