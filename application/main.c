#include <unistd.h>
#include <string.h>
#include "../lib.h"

#define THREADS_MAX 6

int main(int argc, char const *argv[])
{
    /* test code */
    int pipe_in[2];
    int pipe_out[2];

    pipe(pipe_in);
    pipe(pipe_out);

    if (fork() == 0)
    {
        dup2(pipe_in[STDIN_FILENO], STDIN_FILENO);
        dup2(pipe_out[STDOUT_FILENO], STDOUT_FILENO);
        char *const arguments[2] = {"./worker", NULL};
        execv("./worker", arguments);
        _exit(1);
    }

    for (int i = 1; i < argc; i++)
    {
    }
    /* test code */
    return 0;
}
