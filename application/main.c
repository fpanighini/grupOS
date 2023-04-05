#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../lib.h"

#define THREADS_MAX 6
#define BUFFER_SIZE (PATH_MAX * 2)

int main(int argc, char const *argv[])
{
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

    for (int i = 1; i < argc; i++)
    {
        dprintf(pipe_in[WRITE_END], "%s\n", argv[i]);
        getline(&buffer, &n, output);
        printf("%s", buffer);
    }
    free(buffer);
    /* test code */
    return 0;
}
