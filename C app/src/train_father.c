// THIS PROCESS IS CALLED FROM THE MAIN PROCESS
// argv[1] should be either ETCS1 or ETCS2 

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <assert.h>
#include "train_father.h"

#define TRAIN_PROCESS_NAME "train_process"
#define MAx_ROOTPATH "./MAx"


// argv[1]: mode (ETCS1 or ETCS2), argv[2]: map (MAP1 or MAP2), argv[3]: path/to/train_process executable
int main(int argc, char* argv[])
{
    // Assert arguments are correct
    check_arguments(argc, argv);

    // Copy arguments into variables
    char mode[6];
    char map[5];
    char train_process_path[200];
    strncpy(mode, argv[1], 6);
    strncpy(map, argv[2], 5);
    strncpy(train_process_path, argv[3], 200);

    // Create 16 text files, MA1 through MA16, all initialized with "0" inside, with read and write access
    create_ma_files();

    // Create 5 train processes, T1 through T5.
    create_train_processes(mode, map, train_process_path);
}


void check_arguments(int argc, char* argv[])
{
    assert(argc == 4);
    assert(strncmp(argv[1], "ETCS1", 6) == 0 || strncmp(argv[1], "ETCS2", 6) == 0);
    assert(strncmp(argv[2], "MAP1", 5) == 0 || strncmp(argv[2], "MAP2", 6) == 0);
    assert(strncmp(argv[3], "/", 1) == 0); // argv[3] should be a path that points to the train_process executable. This assert is only a small guard
}


void create_ma_files(void)
{
    int filehandle;

    for (int i = 1; i <= 16; i++)
    {
        // Build file name
        char ith_name[100];
        sprintf(ith_name, "%s/MA%d", MAx_ROOTPATH, i);
        umask(0);

        // Open file
        filehandle = open(ith_name, O_WRONLY | O_CREAT, 0777);
        
        // Write "0" to the file. Careful: will only overwrite the 1st character.
        // No reason for the files to have more characters though, and it should not interfere with anything anyway.
        char c = '0';
        write(filehandle, &c, 1);
        close(filehandle);
    }
}


void create_train_processes(char* mode, char* map, char* train_process_path)
{
    char train_name[3];

    pid_t *pids = malloc(5 * sizeof(pid_t));
    for (int i = 0; i < 5; i++)
    {
        // Build train name string (Tx)
        sprintf(train_name, "T%d", i+1);
        
        pid_t pid = fork();

        if (pid < 0)
        {
            printf("Error while creating process %s\n", train_name);
            break;
        }

        else if (pid == 0) // child
        {
            execl(train_process_path, "./bin/train_process", mode, map, train_name, NULL);
            exit(1);  // execl() returns only on error
        }

        else // parent
        {
            pids[i] = pid;
        }

    }

    // Wait for all train_processes
    for (int i = 0; i < 5; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
}