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

#define WORKER_NUMBER 6
#define BUFFER_SIZE (PATH_MAX * 2)

#define WORKER_PATH "./bin/worker"
#define WORKER_NAME "worker"

typedef struct Worker {
    int pipe_in[2];
    int pipe_out[2];
    FILE * fd;
} Worker;



#endif // APPLICATION_H
