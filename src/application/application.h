#ifndef APPLICATION_H
#define APPLICATION_H

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../../lib/lib.h"
#include <sys/select.h>

#include <sys/types.h>
#include <sys/wait.h>

#define WORKERS_MAX 6
#define BUFFER_SIZE (PATH_MAX * 2)

#define WORKER_PATH "./bin/worker"
#define WORKER_NAME "worker"

typedef struct Worker {
    int pid;
    int pipe_write;
    int pipe_read;
    FILE * file_read;
} Worker;


void close_pipes(int pipe_fds[][2], size_t count);
void close_files(FILE * files[], size_t count);

#endif // APPLICATION_H
