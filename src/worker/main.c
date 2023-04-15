// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "../../lib/lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


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
        char *const md5_path = "/usr/bin/md5sum";
        char *const argv[3] = {md5_path, path, NULL};
        execv(md5_path, argv);
        _exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) // wait for child process and ask if status is false and if exit status is not zero
    {                                                                                     // Something went wrong with the child process
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

int main()
{
    char hash[MD5_LEN + 1];
    char buf[SHM_WIDTH];
    char *path = NULL;
    ssize_t read_len;
    size_t n = 0;
    while ((read_len = getline(&path, &n, stdin)) > 0)
    {
        path[read_len - 1] = '\0';

        if (md5sum(hash, path) == -1)
        {
            memset(hash, HASH_ERROR_CHAR, MD5_LEN);
        }
        hash[MD5_LEN] = '\0';

        int write_len = sprintf(buf, WORKER_MESSAGE_FORMAT , getpid(), hash, path);
        if (write(STDOUT_FILENO, &write_len, sizeof(int)) != sizeof(int) || write(STDOUT_FILENO, buf, write_len) != write_len)
        {
            perror("Worker write error");
            free(path);
            return 1;
        }
    }
    int aux = errno;
    free(path);

    if (read_len == -1 && (aux == EINVAL || aux == ENOMEM))
    {
        perror("Worker read error");
        return 1;
    }
    return 0;
}
