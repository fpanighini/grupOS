#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#define MD5_LEN 32
#define PATH_MAX 4096
#define ARG_MAX 2097152
#define FILES_MAX ((ARG_MAX) / (PATH_MAX))

#define READ_END 0
#define WRITE_END 1
#define SHAREMEM_WAIT 2

#define SHARED_MEM_NAME "/shared_memory"
#define SHM_SIZE (10 + MD5_LEN + PATH_MAX) * 10 // RESOLVER!

#define SEMAPHORE_NAME_READ_BUFFER "/shared_memory_semaphore_read_buffer"
#define SEMAPHORE_NAME_VIEWER "/shared_memory_semaphore_viewer"

#define MEM_ERROR (char *) -1


void close_pipe(int pipe_fd[2]);

#endif
