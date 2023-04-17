#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdio.h>
#include <sys/wait.h>
#include "../../lib/lib.h"

#define WORKERS_MAX 6

#define WORKER_PATH "./bin/worker"
#define WORKER_NAME "worker"

#define INITIAL_LOAD 2

#define OUTPUT_PATH "output.txt"

typedef struct Worker
{
    int pid;
    int pipe_write;
    int pipe_read;
    FILE *file_read;
} Worker;

void close_pipes(int pipe_fds[][2], size_t count);
void close_files(FILE *files[], size_t count);

int workers_spawn(Worker workers[], size_t count, fd_set *read_workers);
int workers_free(Worker workers[], size_t count);

#endif // APPLICATION_H
