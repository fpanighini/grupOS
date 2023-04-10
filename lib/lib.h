#ifndef LIB_H
#define LIB_H

#include <unistd.h>

#define MD5_LEN 32
#define PATH_MAX 4096
#define ARG_MAX 2097152
#define FILES_MAX ((ARG_MAX) / (PATH_MAX))

#define READ_END 0
#define WRITE_END 1
#define SHAREMEM_WAIT 2


void close_pipe(int pipe_fd[2]);

#endif
