// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "worker.h"

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

