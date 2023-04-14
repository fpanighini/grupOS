#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

#define MD5_LEN 32
#define PID_LEN 5
#define EXTRA_CHARS 7

#define PATH_MAX 4096

#define READ_END 0
#define WRITE_END 1

#define VIEWER_WAIT 2
#define CANCEL ((char) 24)

#define SHM_WIDTH (MD5_LEN + PATH_MAX + PID_LEN + EXTRA_CHARS)
#define SHM_PATH "/dev/shm/XXXXXX"
#define SHM_PATH_LEN 16

typedef struct SharedMemInfo {
    int file_count;
    char buf_path[SHM_PATH_LEN];
    sem_t sem_viewer;
    sem_t sem_buf;
} SharedMemInfo;

void close_pipe(int pipe_fd[2]);

#endif
