#ifndef APPLICATION_H
#define APPLICATION_H

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../lib.h"
#include <sys/select.h>

#define WORKER_NUMBER 6
#define BUFFER_SIZE (PATH_MAX * 2)

typedef struct Worker {
    int pipe_in[2];
    int pipe_out[2];
    FILE * fd;
} Worker;



#endif // APPLICATION_H
