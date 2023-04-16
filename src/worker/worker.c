// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "worker.h"


int md5sum(char *hash, char *const path)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        return -1;
    }

    int pid = fork();
    if (pid == -1) // Fork Error
    {
        close_pipe(pipe_fd);
        return -1;
    }

    // child code
    if (pid == 0)
    {
        if (dup2(pipe_fd[WRITE_END], STDOUT_FILENO) == -1) // Connect stdout of process with write end of the pipe
        {
            _exit(1);
        }
        close_pipe(pipe_fd);
        char *const argv[3] = {MD5SUM_PATH, path, NULL};
        execv(MD5SUM_PATH, argv);
        _exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)  // wait for child process and ask if status is false and if exit status is not zero
    {                                                                                      // Something went wrong with the child process
        close_pipe(pipe_fd);
        return -1;
    }

    if (read(pipe_fd[READ_END], hash, MD5_LEN) != MD5_LEN)
    {
        close_pipe(pipe_fd);
        return -1;
    }
    close_pipe(pipe_fd);
    return 0;
}
