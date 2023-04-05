#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "../lib.h"
#include <stdio.h>
#include <stdlib.h>

void close_pipe(int pipe_fd[2])
{
    close(pipe_fd[STDOUT_FILENO]);
    close(pipe_fd[STDIN_FILENO]);
}

int md5sum(char *hash, char *const path)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        return -1;
    }

    int pid = fork();
    if (pid == -1)              // Fork Error
    {
        close_pipe(pipe_fd);
        return -1;
    }

    // child code
    if (pid == 0)
    {
        if (dup2(pipe_fd[WRITE_END], STDOUT_FILENO) == -1)          // Connect stdout of process with write end of the pipe
        {
            _exit(1);
        }
        char *const md5_path = "/usr/bin/md5sum";
        char *const argv[3] = {md5_path, path, NULL};
        execv(md5_path, argv);
        _exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)       // wait for child process and ask if status is false and if exit status is not zero
    {                                                                                           // Something went wrong with the child process
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

int main(int argc, char const *argv[])
{

    char * path = NULL;
    char hash[MD5_LEN+1];
    ssize_t len;
    size_t n = 0;
    while ((len = getline(&path,  &n, stdin)) > 0)
    {
        path[len - 1] = 0;
        md5sum(hash, path);
        hash[MD5_LEN] = '\n';

        if (write(STDOUT_FILENO, &hash, MD5_LEN + 1) == -1)
        {
            free(path);
            perror("Worker write");
            return 1;
        }
    }
    if (path != NULL){
        free(path);
    }
    if (len == -1){
        perror("Worker Read");
        return 1;
    }
    return 0;
}
