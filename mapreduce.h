#ifndef _MAPREDUCE_H
#define _MAPREDUCE_H
typedef struct _data_split
{
    int fd; 
    size_t size; 
    void * usr_data;
}DATA_SPLIT;

typedef struct _mapreduce_spec
{
    char * input_data_filepath;
    int split_num;
    int (*map_func)(DATA_SPLIT * split, int fd_out);
    int (*reduce_func)(int * p_fd_in, int fd_in_num, int fd_out);
    void * usr_data;
}MAPREDUCE_SPEC;

typedef struct _mapreduce_result
{
    char * filepath;
    int processing_time;
    int * map_worker_pid;
    int reduce_worker_pid;
}MAPREDUCE_RESULT;

void mapreduce(MAPREDUCE_SPEC * spec, MAPREDUCE_RESULT * result);

#endif
