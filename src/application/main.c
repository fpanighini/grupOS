#include "application.h"

int workers_spawn(Worker workers[], size_t count, fd_set *read_workers)
{
    if (count > WORKERS_MAX)
    {
        return -1;
    }

    // open pipes
    int pipes_path[WORKERS_MAX][2];
    int pipes_hash[WORKERS_MAX][2];
    for (size_t i = 0; i < count; i++)
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
    for (size_t i = 0; i < count; i++)
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
    for (size_t i = 0; i < count; i++)
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
    for (size_t i = 0; i < count; i++)
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
    for (size_t i = 0; i < count; i++)
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

int main(int argc, char const *argv[])
{

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

    fd_set read_workers;
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
    struct timeval t_eval = {0, 100};   // Timeval struct: Time that select will wait
    struct timeval aux_t_eval = t_eval; // Timeval struct: Time that select will wait
    int fd_num;
    char *buffer = NULL;
    size_t n = 0;

    int j;

    fd_set aux = read_workers;

    printf("ARGC: %d, ARGS_R: %d\n", argc, args_read);
    while (args_read < argc)
    {
        for (j = 0; args_read < argc && j < WORKERS_MAX; j++)
        {
            if (getline(&buffer, &n, workers[j].file_read) != -1)
            {
                args_read++;
                printf("%s", buffer);
                if (arg_counter < argc)
                {
                    dprintf(workers[j].pipe_write, "%s\n", argv[arg_counter++]);
                    printf("%d\n", arg_counter);
                    // fd_num--;
                }
            }
        }
        while (args_read < argc && (fd_num = select(workers[WORKERS_MAX - 1].pipe_read + 1, &read_workers, NULL, NULL, &t_eval)) == 0)
        {
            read_workers = aux;
            t_eval = aux_t_eval;
        }
        if (fd_num == -1)
        {
            perror("select");
            workers_free(workers, WORKERS_MAX);
            return 1;
        }
        read_workers = aux;
        t_eval = aux_t_eval;
    }
    free(buffer);
    workers_free(workers, WORKERS_MAX);
    return 0;
}
