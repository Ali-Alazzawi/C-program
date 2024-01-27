#ifndef TRAIN_FATHER_H
#define TRAIN_FATHER_H


// Assert arguments are correct
void check_arguments(int argc, char* argv[]);
// Create 16 text files, MA1 through MA16, all initialized with "0" inside, with read and write access
void create_ma_files(void);
// Create 5 train processes, T1 through T5
void create_train_processes(char* mode, char* map, char* train_process_path);

#endif