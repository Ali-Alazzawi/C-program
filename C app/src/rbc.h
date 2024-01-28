#ifndef RBC_H
#define RBC_H

// Open socket connection with the train processes
void open_rbc_socket(int* sd, int* sd2, int* rc);
// Parse message that the processes sent. Can begin with "ASKMAP" for itinerary request or "REQ" to request a station. 
void parse_read_msg(char msg[1024], char answer[1024]);
// Creates the log/rbc.log file
void create_log_file();
// Logs the passed msg with additional date and map information
void log_msg(char* msg);
// Retrieves an itinerary for a specific train from the register folder
void retrieve_itinerary_str(char* map_path, char* train_name, char itinerary_str[1024]);
// Checks in MAx folder if segment is free
bool is_next_stop_free(char* next_stop, char next_stop_path[200]);
// Changes 0 in 1 / 1 in 0 to act as if a train went to the next segment
void advance_one_stop(char* current_stop, char* next_stop, char* current_stop_path, char* next_stop_path);
// Checks if string starts with MA
bool is_segment(char* stop);
// Close connection with the train processes
void close_server(int sd, int* sd2);

#endif