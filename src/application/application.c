#include "application.h"

void close_pipes(int pipe_fds[][2], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        close_pipe(pipe_fds[i]);
    }
}

void close_files(FILE * files[], size_t count)
{
    for (size_t i = 0; i < count; i++)
    {
        fclose(files[i]);
    }
}
