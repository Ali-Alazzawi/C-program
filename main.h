#ifndef MAIN_H
#define MAIN_H

// Argument handling with assertions. Assigns correct values to mode and map strings.
// 'mode' can be either ETCS1, ETCS2, or RBC. 'map' can be either MAP1 or MAP2.
void check_arguments(int argc, char* argv[], char* mode, char* map);
// Builds file paths for every file binary that will be needed when creating processes 
void create_abspaths(char* argv0, char* train_father_path, char* train_process_path, char* rbc_path);
// Create child processes:
// if mode = RBC: create 'rbc' process
// if mode = ETCS1 or mode = ETCS2: create 'train_father" process
void create_child_processes(char* mode, char* map, char* train_father_path, char* train_process_path, char* rbc_path);

#endif