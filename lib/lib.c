// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "./lib.h"

void close_pipe(int pipe_fd[2])
{
    close(pipe_fd[STDOUT_FILENO]);
    close(pipe_fd[STDIN_FILENO]);
}

int create_shm(char *path_template, SharedMemInfo **shm_info_ref, char **shm_buf_ref, int file_count)
{
    if (file_count <= 0)
    {
        return -1;
    }

    // Returns file descriptor of the file
    int info_fd = mkstemp(path_template);
    if (info_fd == -1)
    {
        return -1;
    }

    // Truncates the file to the size of the shm info struct
    if (ftruncate(info_fd, sizeof(SharedMemInfo)) == -1)
    {
        close(info_fd);
        return -1;
    }

    // Maps & allocates shared memory information
    SharedMemInfo *shm_info = mmap(NULL, sizeof(SharedMemInfo), PROT_READ | PROT_WRITE, MAP_SHARED, info_fd, 0);
    close(info_fd);
    if (shm_info == MAP_FAILED)
    {
        return -1;
    }

    // Initially formats buf_path & returns its file descriptor
    strncpy(shm_info->buf_path, SHM_TEMPLATE, SHM_PATH_LEN);
    int buf_fd = mkstemp(shm_info->buf_path);
    if (buf_fd == -1)
    {
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;

    }

    // Truncates the buffer file for the specific space needed
    if (ftruncate(buf_fd, file_count * SHM_WIDTH) == -1)
    {
        close(buf_fd);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    // Maps & allocates buffer 
    char *shm_buf = mmap(NULL, file_count * SHM_WIDTH, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    close(buf_fd);
    if (shm_buf == MAP_FAILED)
    {
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    // Initialization of viewer semaphore
    shm_info->file_count = file_count;
    if (sem_init(&shm_info->sem_viewer, 1, 1) == -1)
    {
        munmap(shm_buf, file_count * SHM_WIDTH);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    // Initialization of buffer semaphore
    if (sem_init(&shm_info->sem_buf, 1, 0) == -1)
    {
        sem_destroy(&shm_info->sem_viewer);
        munmap(shm_buf, file_count * SHM_WIDTH);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    *shm_info_ref = shm_info;
    *shm_buf_ref = shm_buf;
    return 0;
}

void destroy_shm(SharedMemInfo *shm_info, char *shm_buf)
{
    sem_destroy(&shm_info->sem_buf);
    sem_destroy(&shm_info->sem_viewer);
    munmap(shm_buf, shm_info->file_count * SHM_WIDTH);
    munmap(shm_info, sizeof(SharedMemInfo));
}

int open_shm(const char *path, SharedMemInfo **shm_info_ref, char **shm_buf_ref)
{
    int fd_info = open(path, O_RDWR);
    if (fd_info == -1)
    {
        return -1;
    }

    SharedMemInfo *shm_info = mmap(NULL, sizeof(SharedMemInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_info, 0);
    close(fd_info);
    if (shm_info == MAP_FAILED)
    {
        return -1;
    }

    if (sem_trywait(&shm_info->sem_viewer) == -1)
    {
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    int buf_fd = open(shm_info->buf_path, O_RDONLY);
    if (buf_fd == -1)
    {
        sem_post(&shm_info->sem_viewer);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    char *shm_buf = mmap(NULL, shm_info->file_count * SHM_WIDTH, PROT_READ, MAP_SHARED, buf_fd, 0);
    close(buf_fd);
    if (shm_buf == MAP_FAILED)
    {
        sem_post(&shm_info->sem_viewer);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    *shm_info_ref = shm_info;
    *shm_buf_ref = shm_buf;
    return 0;
}

void close_shm(SharedMemInfo *shm_info, char *shm_buf)
{
    // Deallocates buffer & shm _info shared memory
    munmap(shm_buf, shm_info->file_count * SHM_WIDTH);
    munmap(shm_info, sizeof(SharedMemInfo));
}
