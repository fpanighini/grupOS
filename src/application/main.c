#include "application.h"
#include <unistd.h>
#include <stdlib.h>

int workers_spawn(Worker workers[], size_t count, fd_set *read_workers);

int workers_free(Worker workers[], size_t count);

int main(int argc, char const *argv[])
{
    fd_set read_workers;
    Worker workers[WORKERS_MAX];
    if (workers_spawn(workers, WORKERS_MAX, &read_workers) == -1)
    {
        return 1;
    }

    char info_path[SHM_PATH_LEN] = SHM_PATH;
    int info_fd = mkstemp(info_path);
    if (info_fd == -1){
        perror("mkstemp info");
        return 1;
    }

    if (ftruncate(info_fd, sizeof(SharedMemInfo)) == -1)
    {
        perror("ftruncate info");
        close(info_fd);
        return 1;
    }

    SharedMemInfo * shm_info = mmap(NULL, sizeof(SharedMemInfo), PROT_READ | PROT_WRITE, MAP_SHARED, info_fd, 0);
    close(info_fd);
    if (shm_info == MAP_FAILED)
    {
        perror("mmap info");
        return 1;
    }

    shm_info->file_count = argc - 1;
    strncpy(shm_info->buf_path, SHM_PATH, SHM_PATH_LEN);
    int buf_fd = mkstemp(shm_info->buf_path);
    if (buf_fd == -1){
        perror("mkstemp buf");
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }

    if (ftruncate(buf_fd, (argc - 1) * SHM_WIDTH) == -1)
    {
        perror("ftruncate buf");
        close(buf_fd);
        return 1;
    }

    char * shm_buf = mmap(NULL, shm_info->file_count * SHM_WIDTH, PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0);
    close(buf_fd);
    if (shm_buf == MAP_FAILED){
        perror("mmap buf");
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }

    shm_info->file_count = argc - 1;
    if (sem_init(&shm_info->sem_viewer, 1, 1) == -1)
    {
        munmap(shm_buf, (argc - 1) * SHM_WIDTH);
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }
    if (sem_init(&shm_info->sem_buf, 1, 0) == -1)
    {
        sem_destroy(&shm_info->sem_viewer);
        munmap(shm_buf, (argc - 1) * SHM_WIDTH);
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }
    
    FILE * output_file = fopen(OUTPUT_PATH, "w");
    if (output_file == NULL){
        sem_destroy(&shm_info->sem_viewer);
        sem_destroy(&shm_info->sem_buf);
        munmap(shm_buf, (argc - 1) * SHM_WIDTH);
        munmap(shm_info, sizeof(SharedMemInfo));
        return 1;
    }

    printf("%s", info_path);
    fflush(stdout);
    sleep(VIEWER_WAIT);

    int arg_counter = 1;
    int args_read = 1;
    int i;
    for (i = 0; i < WORKERS_MAX && arg_counter < argc; i++)
    {
        dprintf(workers[i].pipe_write, "%s\n", argv[arg_counter++]);
    }
    int fd_num;
    char *buffer = NULL;
    size_t n = 0;

    int j;

    fd_set aux = read_workers;

    // printf("ARGC: %d, ARGS_R: %d\n", argc, args_read);
    while (args_read < argc)
    {
        while (args_read < argc && (fd_num = select(workers[WORKERS_MAX - 1].pipe_read + 1, &read_workers, NULL, NULL, NULL)) == 0)
        {
            read_workers = aux;
        }
        if (fd_num == -1)
        {
            perror("select");
            workers_free(workers, WORKERS_MAX);
            return 1;
        }
        for (j = 0; args_read < argc && fd_num > 0 && j < WORKERS_MAX; j++)
        {
            if (FD_ISSET(workers[j].pipe_read, &read_workers) && getline(&buffer, &n, workers[j].file_read) != -1)
            {
                args_read++;
                //printf("%s", buffer);
                fprintf(output_file, "%s", buffer);

                // write(shm_fd, buffer, sizeof(buffer));
                // Write to shared mem
                strncpy(shm_buf + ((args_read - 2) * SHM_WIDTH), buffer, SHM_WIDTH);
                sem_post(&shm_info->sem_buf);
                // printf("%d\n", arg_counter);
                if (arg_counter < argc)
                {
                    dprintf(workers[j].pipe_write, "%s\n", argv[arg_counter++]);
                    fd_num--;
                }
            }
        }
        read_workers = aux;
    }
    free(buffer);
    workers_free(workers, WORKERS_MAX);
    fclose(output_file);

    sem_wait(&shm_info->sem_viewer);

    sem_destroy(&shm_info->sem_viewer);
    sem_destroy(&shm_info->sem_buf);
    munmap(shm_buf, (argc - 1) * SHM_WIDTH);
    munmap(shm_info, sizeof(SharedMemInfo));

    return 0;
}

int workers_spawn(Worker workers[], size_t count, fd_set *read_workers)
{
    if (count > WORKERS_MAX)
    {
        return -1;
    }

    // open pipes
    int pipes_path[WORKERS_MAX][2];
    int pipes_hash[WORKERS_MAX][2];
    size_t i;
    for (i = 0; i < count; i++)
    {
        if (pipe(pipes_path[i]) == -1)
        {
            close_pipes(pipes_path, i);
            close_pipes(pipes_hash, i);
            return -1;
        }
        if (pipe(pipes_hash[i]) == -1)
        {
            close_pipes(pipes_path, i + 1);
            close_pipes(pipes_hash, i);
            return -1;
        }
    }

    // fork workers
    int cpid[WORKERS_MAX];
    for (i = 0; i < count; i++)
    {
        cpid[i] = fork();
        if (cpid[i] == -1)
        {
            close_pipes(pipes_path, count);
            close_pipes(pipes_hash, count);
            return -1;
        }

        // child code
        if (cpid[i] == 0)
        {
            if (dup2(pipes_path[i][READ_END], STDIN_FILENO) == -1 || dup2(pipes_hash[i][WRITE_END], STDOUT_FILENO) == -1)
            {
                _exit(1);
            }
            close_pipes(pipes_path, count);
            close_pipes(pipes_hash, count);
            char *const argv[2] = {WORKER_NAME, NULL};
            execv(WORKER_PATH, argv);
            perror("EXECV ERROR");
            _exit(1);
        }
    }

    // open files
    FILE *files_hash[WORKERS_MAX];
    for (i = 0; i < count; i++)
    {
        files_hash[i] = fdopen(pipes_hash[i][READ_END], "r");
        if (files_hash[i] == NULL)
        {
            close_files(files_hash, i);
            close_pipes(pipes_path, count);
            close_pipes(pipes_hash + i, count - i);
            return -1;
        }
    }

    // success: write data
    FD_ZERO(read_workers);
    for (i = 0; i < count; i++)
    {
        close(pipes_path[i][READ_END]);
        close(pipes_hash[i][WRITE_END]);
        workers[i].pid = cpid[i];
        workers[i].pipe_write = pipes_path[i][WRITE_END];
        workers[i].pipe_read = pipes_hash[i][READ_END];
        workers[i].file_read = files_hash[i];
        FD_SET(workers[i].pipe_read, read_workers);
    }

    return 0;
}

int workers_free(Worker workers[], size_t count)
{
    if (count > WORKERS_MAX)
    {
        return -1;
    }

    // close pipes
    size_t i;
    for (i = 0; i < count; i++)
    {
        close(workers[i].pipe_write);
        // fclose(workers[i].file_read);
    }

    // wait for workers to finish
    // for (size_t i = 0; i < count; i++)
    // {
    //     waitpid(workers[i].pid, NULL, 0);
    // }


    return 0;
}
