#ifndef LIB_H
#define LIB_H

#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

#define MD5_LEN 32
#define PID_LEN 5
#define EXTRA_CHARS 7

#define PATH_MAX 4096

#define READ_END 0
#define WRITE_END 1

#define VIEWER_WAIT 2
#define CANCEL ((char) 24)

#define SHM_WIDTH (MD5_LEN + PATH_MAX + PID_LEN + EXTRA_CHARS)
#define SHM_TEMPLATE "/dev/shm/XXXXXX"
#define SHM_PATH_LEN 16

#define WORKER_MESSAGE_FORMAT "%05d - %s - %s\n"
#define HASH_ERROR_CHAR 'X'

typedef struct SharedMemInfo {
    int file_count;
    char buf_path[SHM_PATH_LEN];
    sem_t sem_viewer;
    sem_t sem_buf;
} SharedMemInfo;

void close_pipe(int pipe_fd[2]);

int create_shm(char *path_template, SharedMemInfo **shm_info_ref, char **shm_buf_ref, int file_count);
void destroy_shm(SharedMemInfo *shm_info, char *shm_buf);

int open_shm(const char * path, SharedMemInfo **shm_info_ref, char **shm_buf_ref);
void close_shm(SharedMemInfo *shm_info, char *shm_buf);

#endif
