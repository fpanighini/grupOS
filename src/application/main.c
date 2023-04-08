#include "application.h"

int worker_creator(Worker *worker)
{
    if (pipe(worker->pipe_in) == -1)
    {
        return -1;
    }
    if (pipe(worker->pipe_out))
    {
        close_pipe(worker->pipe_in);
        return -1;
    }

    FILE *output = fdopen(worker->pipe_out[READ_END], "r");
    if (output == NULL)
    {
        close_pipe(worker->pipe_in);
        close_pipe(worker->pipe_out);
        return -1;
    }
    worker->fd = output;
    return 0;
}

void destroy_worker(Worker *worker)
{
    // Destroy individual worker
    close_pipe(worker->pipe_in);
    close_pipe(worker->pipe_out);
    fclose(worker->fd); // Can return error
}

void destroy_workers(Worker *workers, int dim)
{
    int i;
    for (i = 0; i < dim; i++)
    {
        destroy_worker(&workers[i]);
    }
}

int main(int argc, char const *argv[])
{

    if(argc == 1) 
    {
        return 0;
    }

    //int worker_number = 0;
    //if(argc < 5)
    //{
    //  worker_number = argc;
    //} else 
    //{
    //    worker_number = WORKER_NUMBER;
    //}

    fd_set read_workers;
    fd_set write_workers;
    int i;
    Worker workers[WORKER_NUMBER];
    for (i = 0; i < WORKER_NUMBER; i++)
    {
        if (worker_creator(&workers[i]) == -1)
        {
            destroy_workers((Worker *)workers, i);
        }
        if (fork() == 0)
        {
            dup2(workers[i].pipe_in[0], STDIN_FILENO);
            dup2(workers[i].pipe_out[1], STDOUT_FILENO);
            close(workers[i].pipe_in[1]);
            close(workers[i].pipe_out[0]);
            char *const arguments[2] = {WORKER_NAME, NULL};
            execv(WORKER_PATH, arguments);
            _exit(1);
        }
        else
        {
            FD_SET(workers[i].pipe_out[0], &read_workers);
            FD_SET(workers[i].pipe_in[1], &write_workers);
            close(workers[i].pipe_in[0]);
            close(workers[i].pipe_out[1]);
        }
    }

    int arg_counter = 1;
    for (i = 0; i < WORKER_NUMBER && arg_counter < argc; i++)
    {
        write(workers[i].pipe_in[1], argv[arg_counter++], PATH_MAX);
    }
    

    /*
    struct timeval t_eval = {0, 100}; // Timeval struct: Time that select will wait
    int fd_num;
    while (1)
    {
        fd_num = select(WORKER_NUMBER + 1, &read_workers, &write_workers, &read_workers, &t_eval);
        if (fd_num == -1)
        {
            perror("select");
            destroy_workers((Worker **)workers, WORKER_NUMBER);
        }
        while (fd_num > 0)
        {
        }
    }
    */
   
    // /* test code */
    // int pipe_in[2];
    // int pipe_out[2];

    // pipe(pipe_in);
    // pipe(pipe_out);

    // FILE * output = fdopen(pipe_out[READ_END], "r");

    // char * buffer = NULL;
    // size_t n = 0;

    // if (fork() == 0)
    // {
    //     dup2(pipe_in[READ_END], STDIN_FILENO);
    //     dup2(pipe_out[WRITE_END], STDOUT_FILENO);
    //     char *const arguments[2] = {"worker", NULL};
    //     execv("./worker", arguments);
    //     _exit(1);
    // }
    // for (i = 1; i < argc; i++)
    // {
    //     dprintf(pipe_in[WRITE_END], "%s\n", argv[i]);
    //     getline(&buffer, &n, output);
    //     printf("%s", buffer);
    //     // Write to shared mem
    // }
    // free(buffer);
    // /* test code */

    destroy_workers((Worker *)workers, WORKER_NUMBER);

    return 0;
}
