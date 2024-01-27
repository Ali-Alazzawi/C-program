#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include "rbc.h"

#define BUFFER_LENGTH 250
#define FALSE 0
#define FILENAME "rbc"
#define SERVER_PATH "/tmp/rbc_server"

char root_path[150];
char logs_path[200];
char log_str[200];
char map[5];

int main(int argc, char* argv[])
{
   char buffer[1024];
   char answer[1024];
   int amount_read, sd=-1, rc=-1;
   int sd2[5] = {-1, -1, -1, -1, -1};
   bool data_was_read;

   // TODO signal handler - must make the server vars global
   // signal(SIGINT, close_server(sd, sd2))

   // Copy map argument into 'map'
   assert(strncmp(argv[1], "MAP", 3) == 0);
   strcpy(map, argv[1]);

   // Build absolute path to the root project folder 
   realpath(argv[0], root_path);
   root_path[strlen(root_path) - strlen(FILENAME) - 1] = '\0';

   // Build abspath for the logging file, then create it
   sprintf(logs_path, "%s/logs/rbc.log", root_path);
   create_log_file();

   // Open the server connection for each train process
   open_rbc_socket(&sd, sd2, &rc);

   // setup server API main loop
   while(true)
   {
      data_was_read = false;

      // iterate through all the train processes that are connected to the socket
      for (int i = 0; i < 5; i++)
      {
         // read request into 'buffer'
         amount_read = recv(sd2[i], buffer, 1024, 0);
         
         // Don't parse the message if there wasn't any
         if (amount_read == 0)
            continue;

         // Indicates that the train processes are still sending messages
         data_was_read = true;
         
         // process the message that was just read. Put the answer to send into 'answer'
         parse_read_msg(buffer, answer);
         send(sd2[i], answer, strlen(answer)+1, 0);
      }

      // Break when no data was read from any of the train processes
      if (!data_was_read)
      {
         break;
      }
   }

   sprintf(log_str, "[Finished job. Closing server]");
   log_msg(log_str);

   printf("Finished operations.\n");
   
   close_server(sd, sd2);
}


void open_rbc_socket(int* sd, int* sd2, int* rc)
{
   // TODO ce mode entier
   struct sockaddr_un serveraddr;

   // socket creation
   *sd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (*sd < 0)
   {
      printf("socket() failed");
      close_server(*sd, sd2);
   }

   // set server address
   memset(&serveraddr, 0, sizeof(serveraddr));
   serveraddr.sun_family = AF_UNIX;
   strcpy(serveraddr.sun_path, SERVER_PATH);

   // bind it
   *rc = bind(*sd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));
   if (*rc < 0)
   {
      printf("bind() failed");
      close_server(*sd, sd2);
   }

   // listen for up to 5 connections before denying 
   *rc = listen(*sd, 5);
   if (*rc< 0)
   {
      printf("listen() failed");
      close_server(*sd, sd2);
   }

   sprintf(log_str, "[Waiting for the trains to connect]");
   log_msg(log_str);

   // blocks until the 5 train process connections are found
   for (int i = 0; i < 5; i++)
   {

      sd2[i] = accept(*sd, NULL, NULL);

      if (*sd2 < 0)
      {
         printf("accept() failed for train number %d", i+1);
         close_server(*sd, sd2);
      }
   }

   sprintf(log_str, "[All connections established]");
   log_msg(log_str);
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
   fprintf(log_file, "[ETCS2] [%s] %-45s (time: %02d:%02d:%02d, date: %02d/%02d/%02d)\n",
           map, msg, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900);
   fclose(log_file);
}


void parse_read_msg(char msg[1024], char answer[1024])
{
   // if request itinerary, send it as str
   // if request segment access, send answer ('1' or '0')
   // if request station access, send '1'

   if (strncmp(msg, "ASKMAP ", 7) == 0) // itinerary_str request
   {
      int train_num;
      char map_path[200], train_name[5];

      sscanf(msg, "ASKMAP T%d", &train_num); // Parse request arguments
      sprintf(map_path, "%s/register/%s.txt", root_path, map); // Build map file path
      sprintf(train_name, "T%d", train_num);
      retrieve_itinerary_str(map_path, train_name, answer); // Look for the itinerary string
   }
   else if(strncmp(msg, "REQ ", 4) == 0) // Request a sstation
   {
      char* current_stop = NULL;
      char* next_stop = NULL;
      char* train_name = NULL;
      char current_stop_path[200];
      char next_stop_path[200];

      // Retrieve the stop arguments in the msgs tring
      strtok(msg, " ");
      train_name = strtok(NULL, " ");
      current_stop = strtok(NULL, " ");
      next_stop = strtok(NULL, " ");

      // Build file paths (which may not exist if a stop is a station - but we will only open the files if they are not)
      sprintf(current_stop_path, "%s/MAx/%s", root_path, current_stop);
      sprintf(next_stop_path, "%s/MAx/%s", root_path, next_stop);
      
      if (is_next_stop_free(next_stop, next_stop_path))
      {
         advance_one_stop(current_stop, next_stop, current_stop_path, next_stop_path);
         
         sprintf(log_str, "[%s] [FROM: %s, REQ: %s] [ACCEPTED: YES]", train_name, current_stop, next_stop);
         log_msg(log_str);
         
         strcpy(answer, "1");
      }
      else
      {
         sprintf(log_str, "[%s] [FROM: %s, REQ: %s] [ACCEPTED: NO]", train_name, current_stop, next_stop);
         log_msg(log_str);

         strcpy(answer, "0");
      }

   }
   else
   {
      printf("Couldn't parse message: %s\n", msg);
      strcpy(answer, "-1");
   }
}

bool is_next_stop_free(char* next_stop, char next_stop_path[200])
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

void advance_one_stop(char* current_stop, char* next_stop, char* current_stop_path, char* next_stop_path)
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
}



bool is_segment(char* stop)
{
    return (strncmp(stop, "MA", 2) == 0);
}


void retrieve_itinerary_str(char* map_path, char* train_name, char itinerary_str[1024])
{
   // Open map register file
   FILE* map_file = fopen(map_path, "r");
   if (map_file == NULL)
   {
         printf("Error while opening %s in the register\n", map_path);
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


void close_server(int sd, int* sd2)
{
   printf("Closing server...\n");
   if (sd != -1)
      close(sd);

   for (int i = 0; i < 5; i++)
   {
      if (sd2[i] != -1)
         close(sd2[i]);
   }
   
   unlink(SERVER_PATH);
   exit(0);
}
