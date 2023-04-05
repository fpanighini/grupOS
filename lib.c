#include "lib.h"

void close_pipe(int pipe_fd[2])
{
    close(pipe_fd[STDOUT_FILENO]);
    close(pipe_fd[STDIN_FILENO]);
}

