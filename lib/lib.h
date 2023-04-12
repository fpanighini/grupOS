#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#define MD5_LEN 32
#define PID_LEN 5
#define EXTRA_CHARS 7
#define PATH_MAX 4096
#define ARG_MAX 2097152
#define FILES_MAX ((ARG_MAX) / (PATH_MAX))

#define READ_END 0
#define WRITE_END 1
#define SHAREMEM_WAIT 2

#define SHARED_MEM_NAME "/shared_memory"
#define SHARED_MEM_BUF_NAME "/shared_memory_buf"
#define SHM_WIDTH (MD5_LEN + PATH_MAX + PID_LEN + EXTRA_CHARS)

#define SEMAPHORE_NAME_READ_BUFFER "/shared_memory_semaphore_read_buffer"
#define SEMAPHORE_NAME_VIEWER "/shared_memory_semaphore_viewer"

#define MEM_ERROR ((char *) -1)

typedef struct Shm_s {
    int * size;
    char * buf;
} Shm_t;

void close_pipe(int pipe_fd[2]);

#endif
