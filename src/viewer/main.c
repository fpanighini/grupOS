// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "viewer.h"
#include <stdio.h>

int main(int argc, char const *argv[])
{
    char shm_path[SHM_PATH_LEN];
    if (argc < 2)
    {
        read(0, shm_path, SHM_PATH_LEN);
    }
    else if (argc == 2)
    {
        strncpy(shm_path, argv[1], SHM_PATH_LEN);
    }
    else
    {
        fprintf(stderr, "Too many arguments\n");
        return PARAMETER_ERROR;
    }

    shm_path[SHM_PATH_LEN - 1] = '\0';

    SharedMemInfo *shm_info;
    char *shm_buf;
    if (open_shm(shm_path, &shm_info, &shm_buf) == -1)
    {
        fprintf(stderr, "Failed to open shared memory\n");
        return MEMORY_ERROR;
    }

    // Waits for the results in shm_buf and prints them to standard output
    for (int i = 0; i < shm_info->file_count; i++)
    {
        sem_wait(&shm_info->sem_buf);
        dprintf(STDOUT_FILENO, "%s", shm_buf + (i * SHM_WIDTH));
    }

    // Marks end
    sem_post(&shm_info->sem_viewer);

    close_shm(shm_info, shm_buf);

    return SUCCESS;
}
