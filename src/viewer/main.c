#include "viewer.h"
#include <unistd.h>

int main(int argc, char const *argv[])
{
    int fd_info;
    if (argc < 2){
        char shm_path[SHM_PATH_LEN];
        read(0, shm_path, SHM_PATH_LEN);
        shm_path[SHM_PATH_LEN - 1] = '\0';
        fd_info = open(shm_path, O_RDWR);
    }
    else if (argc == 2) {
        fd_info = open(argv[1], O_RDWR);
    }
    else
    {
        perror("Incorrect number of arguments");
        return 1;
    }

    if (fd_info == -1){
        perror("open info");
        return -1;
    }

    SharedMemInfo * shm_info = mmap(NULL, sizeof(SharedMemInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_info, 0);
    close(fd_info);
    if (shm_info == MAP_FAILED)
    {
        perror("mmap info");
        close(fd_info);
        return 1;
    }

    if (sem_trywait(&shm_info->sem_viewer) == -1)
    {
        perror("application unavailable");
        close(fd_info);
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }

    int buf_fd = open(shm_info->buf_path, O_RDONLY);
    if (buf_fd == -1){
        perror("open buf");
        close(fd_info);
        munmap(shm_info, sizeof(SharedMemInfo));
        return -1;
    }

    char * shm_buf = mmap(NULL, shm_info->file_count * SHM_WIDTH, PROT_READ, MAP_SHARED, buf_fd, 0);
    close(buf_fd);
    if (shm_buf == MAP_FAILED){
        perror("mmap buf");
        close(buf_fd);
        close(fd_info);
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }

    for (int i = 0; i < shm_info->file_count; i++)
    {
        sem_wait(&shm_info->sem_buf);
        dprintf(STDOUT_FILENO, "%s", shm_buf + (i * SHM_WIDTH));
    }

    sem_post(&shm_info->sem_viewer);

    close(buf_fd);
    close(fd_info);
    munmap(shm_buf,shm_info->file_count * SHM_WIDTH);
    munmap(shm_info, sizeof(SharedMemInfo));

    return 0;
}
