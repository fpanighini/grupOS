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

    // Fork Error
    if (pid == -1)
    {
        close_pipe(pipe_fd);
        return -1;
    }

    // child code
    if (pid == 0)
    {
        // Connect stdout of process with write end of the pipe
        if (dup2(pipe_fd[WRITE_END], STDOUT_FILENO) == -1)
        {
            _exit(1);
        }
        close_pipe(pipe_fd);
        char *const argv[3] = {MD5SUM_PATH, path, NULL};
        execvp(MD5SUM_PATH, argv);
        _exit(1);
    }

    int status;
    // Wait for child process, check if status is false and check exit status
    if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        // Something went wrong with the child process
        close_pipe(pipe_fd);
        return -1;
    }

    // Read hashes
    if (read(pipe_fd[READ_END], hash, MD5_LEN) != MD5_LEN)
    {
        close_pipe(pipe_fd);
        return -1;
    }
    close_pipe(pipe_fd);
    return 0;
}
