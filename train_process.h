#ifndef TRAIN_PROCESS_H
#define TRAIN_PROCESS_H

#include "train_process_utils.h"


// Check passed arguments
void check_arguments(int argc, char* argv[]);
// Creates a log/Tx.log file
void create_log_file();
// Log msg into the log/Tx.log files, with datetime/Mode/Map added
void log_msg(char* msg);
// Checks if the stop starts with MA
bool is_segment(char* stop);
// Parses an itinerary string of the form that is found inside register/MAPx.txt, into a linked list
List* convert_itinerary_str_to_list(char itinerary_str[1024]);


// ETCS1 specific functions -------------------

// Retrieve itinerary string from inside the register/MAPx.txt files
void etcs1_retrieve_itinerary_str(char* map_path, char itinerary_str[1024]);
// Checks if the MAx/MAx file corresponding to next_stop is free or not (0 or 1)
bool etcs1_is_next_stop_free(List* itinerary, char* next_stop, char next_stop_path[200]);
// Changes 0 in 1 / 1 in 0 to act like the train went to the next segment. Skips to the next step in the itinerary linked list
void etcs1_advance_one_stop(List* itinerary, char* current_stop, char* next_stop, char* current_stop_path, char* next_stop_path);

// --------------------------------------------


// ETCS2 specific functions -------------------

// Open the socket connection with the RBC server
void etcs2_open_rbc_socket(char* server_path, int* sd, int* rc);

// --------------------------------------------


#endif