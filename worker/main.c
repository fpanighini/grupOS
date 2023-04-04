#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "../lib.h"

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
    if (pid == -1)
    {
        close_pipe(pipe_fd);
        return -1;
    }

    // child code
    if (pid == 0)
    {
        if (dup2(pipe_fd[STDOUT_FILENO], STDOUT_FILENO) == -1)
        {
            _exit(1);
        }
        char *const md5_path = "/usr/bin/md5sum";
        char *const argv[3] = {md5_path, path, NULL};
        execv(md5_path, argv);
        _exit(1);
    }

    int status;
    if (waitpid(pid, &status, 0) == -1 || !WIFEXITED(status) || WEXITSTATUS(status) != 0)
    {
        close_pipe(pipe_fd);
        return -1;
    }

    if (read(pipe_fd[STDIN_FILENO], hash, MD5_LEN) != MD5_LEN)
    {
        close_pipe(pipe_fd);
        return -1;
    }

    close_pipe(pipe_fd);
    return 0;
}

int main(int argc, char const *argv[])
{
    Result result;
    Task task;

    while (1)
    {
        int bytes = read(STDIN_FILENO, &task, sizeof(Task));
        if (bytes == 0)
        {
            return 0;
        }
        else if (bytes != sizeof(Task))
        {
            return 1;
        }

        if (md5sum(result.hash, task.path) == -1)
        {
            memset(result.hash, '0', MD5_LEN);
        }

        strncpy(result.path, task.path, PATH_MAX);

        if (write(STDOUT_FILENO, &result, sizeof(Result)) != sizeof(Result))
        {
            return 1;
        }
    }
}
