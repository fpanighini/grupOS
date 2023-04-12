#include "application.h"
#include <unistd.h>

int workers_spawn(Worker workers[], size_t count, fd_set *read_workers);

int workers_free(Worker workers[], size_t count);


int main(int argc, char const *argv[])
{

    const char* shm_name = SHARED_MEM_NAME;

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1){
        perror("shm_open");
        return -1;
    }

    if (ftruncate(shm_fd, SHM_SIZE) == -1){
        perror("ftruncate");
        return -1;
    }

    char *shared_mem = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    sem_t *sem = sem_open(SEMAPHORE_NAME_READ_BUFFER, O_CREAT | O_RDWR, 0666, 0);
    sem_t *sem_viewer = sem_open(SEMAPHORE_NAME_VIEWER, O_CREAT | O_RDWR, 0666, 0);
    sem_post(sem_viewer);
    /*
       int worker_number = 0;
       if(argc < 5)
       {
       worker_number = argc;
       } else
       {
       worker_number = WORKER_NUMBER;
       }
       */
    // Shared memory creation
    sleep(SHAREMEM_WAIT);

    fd_set read_workers;
    FD_ZERO(&read_workers);
    int i;
    Worker workers[WORKERS_MAX];
    if (workers_spawn(workers, WORKERS_MAX, &read_workers) == -1)
    {
        return 1;
    }

    int arg_counter = 1;
    int args_read = 1;
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
                printf("%s", buffer);

                // write(shm_fd, buffer, sizeof(buffer));
                sem_post(sem);
                // Write to shared mem
                // shm_mem[arg_counter] = malloc(strlen(buffer) * sizeof(char));
                // strcpy(buffer, shm_mem[arg_counter++]);
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

    sem_wait(sem_viewer);
    sem_close(sem);
    sem_unlink(SEMAPHORE_NAME_READ_BUFFER);
    sem_close(sem_viewer);
    sem_unlink(SEMAPHORE_NAME_VIEWER);




    if(munmap(shared_mem, SHM_SIZE) == -1){
        perror("munmap");
        return -1;
    }
    if(shm_unlink(SHARED_MEM_NAME) == -1){
        perror("shm_unlink");
        return -1;
    }

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
            close_pipes(pipes_hash, count);
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
        fclose(workers[i].file_read);
    }

    // wait for workers to finish
    // for (size_t i = 0; i < count; i++)
    // {
    //     waitpid(workers[i].pid, NULL, 0);
    // }


    return 0;
}
