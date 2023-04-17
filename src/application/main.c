// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "application.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MIN(x, y) ((x) < (y) ? (x) : (y))

int clamp(int x, int min, int max);

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        dprintf(STDERR_FILENO, "Not Enough Arguments\n");
        return 1;
    }

    fd_set read_workers;
    Worker workers[WORKERS_MAX];
    size_t workers_load[WORKERS_MAX] = {0};
    size_t workers_count = clamp((argc - 1) / INITIAL_LOAD, 1, WORKERS_MAX);
    if (workers_spawn(workers, workers_count, &read_workers) == -1)
    {
        return 1;
    }

    char info_path[SHM_PATH_LEN] = SHM_TEMPLATE;
    SharedMemInfo *shm_info;
    char *shm_buf;
    if (create_shm(info_path, &shm_info, &shm_buf, argc - 1) == -1)
    {
        workers_free(workers, workers_count);
        return 1;
    }

    FILE *output_file = fopen(OUTPUT_PATH, "w");
    if (output_file == NULL)
    {
        destroy_shm(shm_info, shm_buf);
        workers_free(workers, workers_count);
        return 1;
    }

    dprintf(STDOUT_FILENO, "%s", info_path);
    sleep(VIEWER_WAIT);

    int arg_i;
    int initial_count = MIN((int) workers_count * INITIAL_LOAD, (argc - 1));
    for (arg_i = 1; arg_i < (initial_count + 1); arg_i++)
    {
        int worker_id = (arg_i - 1) / INITIAL_LOAD;
        dprintf(workers[worker_id].pipe_write, "%s\n", argv[arg_i]);
        workers_load[worker_id]++;
    }

    size_t shm_i = 0;
    char *buffer = NULL;
    size_t n = 0;
    int workers_active = workers_count;
    while (workers_active > 0)
    {
        fd_set result = read_workers;
        int fds_available = select(workers[workers_count - 1].pipe_read + 1, &result, NULL, NULL, NULL);
        if (fds_available == -1)
        {
            perror("select");
            fclose(output_file);
            destroy_shm(shm_info, shm_buf);
            workers_free(workers, workers_count);
            return 1;
        }
        size_t i;
        for (i = 0; i < workers_count && fds_available > 0; i++)
        {
            if (FD_ISSET(workers[i].pipe_read, &result))
            {
                int chars_read = getline(&buffer, &n, workers[i].file_read);
                if (chars_read == -1)
                {
                    perror("read");
                    free(buffer);
                    fclose(output_file);
                    destroy_shm(shm_info, shm_buf);
                    workers_free(workers, workers_count);
                    return 1;
                }
                workers_load[i]--;

                fprintf(output_file, "%s", buffer);

                strncpy(shm_buf + (shm_i * SHM_WIDTH), buffer, chars_read);
                shm_i++;
                sem_post(&shm_info->sem_buf);

                if (workers_load[i] == 0)
                {
                    if (arg_i < argc)
                    {
                        dprintf(workers[i].pipe_write, "%s\n", argv[arg_i]);
                        arg_i++;
                        workers_load[i]++;
                    }
                    else
                    {
                        FD_CLR(workers[i].pipe_read, &read_workers);
                        workers_active--;
                    }
                }
                fds_available--;
            }
        }
    }
    free(buffer);

    fclose(output_file);

    sem_wait(&shm_info->sem_viewer);

    destroy_shm(shm_info, shm_buf);
    workers_free(workers, workers_count);

    return 0;
}

int clamp(int x, int min, int max)
{
    if (x < min)
    {
        return min;
    }
    if (x > max)
    {
        return max;
    }
    return x;
}
