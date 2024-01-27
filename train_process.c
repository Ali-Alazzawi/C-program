#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>
#include "train_process.h"
#include "train_process_utils.h"

#define PORT 8080
#define SERVER_PATH "/tmp/rbc_server"
#define BUFFER_LENGTH    250
#define FILENAME "bin/train_process"


// Global declarations
char mode[6];
char map[5];
char train_name[6];
char log_str[200];
char itinerary_str[1024];
char logs_path[200];


// argv[1]: mode (ETCS1 or ETCS2), argv[2]: map (MAP1 or MAP2), argv[3]: Tx (train name)
int main(int argc, char* argv[])
{
    // Assert arguments are correct
    check_arguments(argc, argv);

    // Copy args into variables
    strncpy(mode, argv[1], 6);
    strncpy(map, argv[2], 5);
    strncpy(train_name, argv[3], 3);

    char* current_stop = NULL;
    char* next_stop = NULL;

    // Build absolute path to the root project folder 
    char root_path[150];
    realpath(argv[0], root_path);
    root_path[strlen(root_path) - strlen(FILENAME) - 1] = '\0';

    // Build abspath for the logging file, then create it
    sprintf(logs_path, "%s/logs/%s.log", root_path, train_name);
    create_log_file();


    // ETCS1 train_process implementation
    if (strncmp(mode, "ETCS1", 6) == 0)
    {
        // ETCS1-specific variables declarations
        char map_path[200];
        char MAx_path[200];


        // Build abspaths for the register/MAPx.txt file and the MAx folder
        sprintf(map_path, "%s/register/%s.txt", root_path, map);
        sprintf(MAx_path, "%s/MAx", root_path);

        // Retrieves the itinerary string from the MAPx.txt file and puts it into itinerary_str
        etcs1_retrieve_itinerary_str(map_path, itinerary_str);

        // Convert 'itinerary_str' to a linked list of each itinerary step
        List* itinerary = convert_itinerary_str_to_list(itinerary_str);

        // Check for empty itinerary
        if (itinerary->head == NULL)
            return 0;

        // Will store paths to the different segment files (MAx)
        char current_stop_path[250];
        char next_stop_path[250];

        // Begin log message
        sprintf(log_str, "[BEGIN at station %s]", itinerary->head->item);
        log_msg(log_str);

        // Main loop: runs every 2 seconds, as long as there are elements left in the itinerary linked list
        while (itinerary->head->next != NULL)
        {
           
            int train_number = (int)(train_name[1] - 48);
            usleep(train_number * 2);

            // Build absolute paths for the current stop and next stop
            current_stop = itinerary->head->item;
            next_stop = itinerary->head->next->item;
            sprintf(current_stop_path, "%s/%s", MAx_path, current_stop);
            sprintf(next_stop_path, "%s/%s", MAx_path, next_stop);

            // Log current position
            sprintf(log_str, "[CURRENT: %4s; NEXT: %4s]", current_stop, next_stop);
            log_msg(log_str);

            // Check if next stop is free
            if (etcs1_is_next_stop_free(itinerary, next_stop, next_stop_path))
            {
                // If it is, go to the next stop
                etcs1_advance_one_stop(itinerary, current_stop, next_stop, current_stop_path, next_stop_path);
            }

            // Sleep is equal to 2 seconds for any train process
            usleep(2 - train_number*2);
        }

        // log when main loop is finished
        sprintf(log_str, "[END at station %s]", itinerary->head->item);
        log_msg(log_str);
    }


    // ETCS2 train_process implementation
    else if (strncmp(mode, "ETCS2", 6) == 0)
    {
        char server_req[1024];
        char server_ans[1024];

        // Build path to the server executable
        char rbc_path[200];
        sprintf(rbc_path, "%s/bin/rbc", root_path);

        // Establish a socket connection to the server
        int rc = -1, sd = -1;
        etcs2_open_rbc_socket(rbc_path, &sd, &rc);

        // Ask the rbc server to return an itinerary string
        sprintf(server_req, "ASKMAP %s", train_name);
        send(sd, server_req, 1024, 0);
        recv(sd, itinerary_str, 1024, 0);

        // Convert response to a linked list
        List* itinerary = convert_itinerary_str_to_list(itinerary_str);

        // Check for empty itinerary
        if (itinerary->head == NULL)
            return 0;

        // Begin log message
        sprintf(log_str, "[BEGIN at station %s]", itinerary->head->item);
        log_msg(log_str);

        // Main loop
        while(itinerary->head->next != NULL)
        {
            // Retrive curr/next stop from list
            current_stop = itinerary->head->item;
            next_stop = itinerary->head->next->item;

            // Log current position
            sprintf(log_str, "[CURRENT: %4s; NEXT: %4s]", current_stop, next_stop);
            log_msg(log_str);

            // Ask the rbc server to return "1" or "0" for access to the next station
            sprintf(server_req, "REQ %s %s %s", train_name, current_stop, next_stop);
            send(sd, server_req, 1024, 0);
            sleep(2);
            recv(sd, server_ans, 1024, 0);

            // If server authorized access to the next station, we advance 1 stop
            if (strncmp(server_ans, "1", 1) == 0)
                del_from_head(itinerary);
        }

        // log when main loop is finished
        sprintf(log_str, "[END at station %s]", itinerary->head->item);
        log_msg(log_str);


        // Close down the socket connection
        if (sd != -1)
            close(sd);
    }

    printf("[%s] Finished operations.\n", train_name);
    return 0;
}


void check_arguments(int argc, char* argv[])
{
    assert(argc == 4);
    assert(strncmp(argv[1], "ETCS1", 6) == 0 || strncmp(argv[1], "ETCS2", 6) == 0);
    assert(strncmp(argv[2], "MAP1", 6) == 0 || strncmp(argv[2], "MAP2", 6) == 0);
    assert(strncmp(argv[3], "T", 1) == 0); // this only asserts that argv[2] begins by, T, but it should be Tx (e.g T1, T2, T3...)
}


void create_log_file()
{
    // Create logging file only once
	static bool file_created = false;
	if (!file_created)
	{
		int filehandle = open(logs_path, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	    close(filehandle);
		file_created = true;
	}
}


void log_msg(char* msg)
{
	// Log msg with datetime
	time_t datetime = time(NULL);
    struct tm *ptm = localtime(&datetime);

    FILE* log_file = fopen(logs_path, "a");
    fprintf(log_file, "[%s] [%s] %-27s (time: %02d:%02d:%02d, date: %02d/%02d/%02d)\n",
            mode, map, msg, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
    fclose(log_file);
}


List* convert_itinerary_str_to_list(char itinerary_str[1024])
{
    List* itinerary = malloc(sizeof(List));
    init_list(itinerary);

    char* str_ptr = itinerary_str;

    // First skip the 3 first characters (train name)
    str_ptr += 3;
    
    for (;;) 
    {

        int temp_int = 0;
        char* temp_str = NULL;
        
        // if instruction is END: end of the itinerary string
        if (strncmp(str_ptr, "END", 3) == 0)
        {
            break;
        }

        // if instruction begins by S: we add it the entire station name as a node in the linked list
        else if (strncmp(str_ptr, "S", 1) == 0)
        {
            char* new_data = malloc(5 * sizeof(char));
            sscanf(str_ptr, "S%d %s", &temp_int, temp_str);
            sprintf(new_data, "S%d", temp_int);
            str_ptr += strlen(new_data) + 1;
            add_at_tail(itinerary, new_data);
        }
        // if instruction begins by M: we add it the entire segment name as a node in the linked list
        else if (strncmp(str_ptr, "MA", 2) == 0)
        {
            char* new_data = malloc(5 * sizeof(char));
            sscanf(str_ptr, "MA%d %s", &temp_int, temp_str);
            sprintf(new_data, "MA%d", temp_int);
            str_ptr += strlen(new_data) + 1;
            add_at_tail(itinerary, new_data);
        }
        else
        {
            printf("Error: incorrectly formatted itinerary string\n");
            exit(EXIT_FAILURE);
        }
    }
    return itinerary;
}


bool is_segment(char* stop)
{
    return (strncmp(stop, "MA", 2) == 0);
}


void etcs1_retrieve_itinerary_str(char* map_path, char itinerary_str[1024])
{
    // Open map register file
    FILE* map_file = fopen(map_path, "r");
    if (map_file == NULL)
    {
        printf("Error while opening %s in the register\n", map);
        exit(EXIT_FAILURE);
    }

    bool found_itinerary = false;

    // Read 1 line of map file at a time, until the correct itinerary for this train process is found. Store it in itinerary_str
    while(fgets(itinerary_str, 200, map_file) != NULL) // Capture 1 line into itinerary_str
    {
        // Stop looping when the line begins by the correct train name (Tx).
        if (strncmp(itinerary_str, train_name, 2) == 0)
        {
            found_itinerary = true;
            break;
        }
    }
    fclose(map_file);

    if (found_itinerary == false)
    {
        printf("Couldn't find the itinerary for train %s in file %s", train_name, map_path);
        exit(EXIT_FAILURE);
    }
}


bool etcs1_is_next_stop_free(List* itinerary, char* next_stop, char next_stop_path[200])
{
    // check if next stop is a station: if yes, then access is granted (return true).
    if (!is_segment(next_stop))
        return true;

    // if not, then open file corresponding to the next stop. read 1 character. If it's '0', access is granted (return true)
    FILE* next_stop_file = fopen(next_stop_path, "r");
    char c;
    c = getc(next_stop_file);
    fclose(next_stop_file);
    if (c == '0') return true;
    else          return false;
}


void etcs1_advance_one_stop(List* itinerary, char* current_stop, char* next_stop, char* current_stop_path, char* next_stop_path)
{
    if (is_segment(current_stop))
    {
        // put '0' inside the current_stop_path file
        FILE* current_stop_file = fopen(current_stop_path, "w");
        fputc('0', current_stop_file);
        fclose(current_stop_file);
    }
    if (is_segment(next_stop))
    {
        // put '1' inside the next_stop_path file
        FILE* next_stop_file = fopen(next_stop_path, "w");
        fputc('1', next_stop_file);
        fclose(next_stop_file);
    }

    // Advance one step in the linked list
    del_from_head(itinerary);
}


void etcs2_open_rbc_socket(char* server_path, int* sd, int* rc)
{
    
    // Variable and structure definitions.
    struct sockaddr_un serveraddr;

    // Create socket
    *sd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (*sd < 0)
    {
        printf("socket() failed\n");
        close(*sd);
        exit(EXIT_FAILURE);
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sun_family = AF_UNIX;
    strcpy(serveraddr.sun_path, SERVER_PATH);

    sprintf(log_str, "[Waiting for the RBC server]");
    log_msg(log_str);

    // Use the connect() function to establish a connection to the server.
    while (*rc < 0)
    {
        *rc = connect(*sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
    }

    sprintf(log_str, "[Connection established]");
    log_msg(log_str);
}
