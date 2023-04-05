#include "application.h"

int worker_creator(Worker * worker){
    if (pipe(worker->pipe_in) == -1){
        return -1;
    }
    if (pipe(worker->pipe_out)){
        close_pipe(worker->pipe_in);
        return -1;
    }

    FILE * output = fdopen(worker->pipe_out[READ_END], "r");
    if (output == NULL){
        close_pipe(worker->pipe_in);
        close_pipe(worker->pipe_out);
        return -1;
    }
    return 0;
}


void destroy_worker(Worker * worker){
    // Destroy individual worker
}

void destroy_workers(Worker  * workers, int dim){
    int i;
    for (i = 0 ; i < dim ; i++){
        destroy_worker(&workers[i]);
    }
}


int main(int argc, char const *argv[])
{
    fd_set read_workers;
    int i;
    Worker workers[WORKER_NUMBER];
    for (i = 0 ; i < WORKER_NUMBER ; i++){
        if (worker_creator(&workers[i]) == -1){
            destroy_workers(workers, i);
        }
        FD_SET(workers[i].pipe_out[0], &read_workers);
    }





    /* test code */
    int pipe_in[2];
    int pipe_out[2];

    pipe(pipe_in);
    pipe(pipe_out);

    FILE * output = fdopen(pipe_out[READ_END], "r");

    char * buffer = NULL;
    size_t n = 0;

    if (fork() == 0)
    {
        dup2(pipe_in[READ_END], STDIN_FILENO);
        dup2(pipe_out[WRITE_END], STDOUT_FILENO);
        char *const arguments[2] = {"worker", NULL};
        execv("./worker", arguments);
        _exit(1);
    }
    for (i = 1; i < argc; i++)
    {
        dprintf(pipe_in[WRITE_END], "%s\n", argv[i]);
        getline(&buffer, &n, output);
        printf("%s", buffer);
        // Write to shared mem
    }
    free(buffer);
    /* test code */
    return 0;
}
