// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "worker.h"

int main()
{
    char hash[MD5_LEN + 1];
    char *path = NULL;
    ssize_t read_len;
    size_t n = 0;
    while ((read_len = getline(&path, &n, stdin)) > 0)
    {
        path[read_len - 1] = '\0';

        // Performs hash
        if (md5sum(hash, path) == -1)
        {
            memset(hash, HASH_ERROR_CHAR, MD5_LEN);
        }
        hash[MD5_LEN] = '\0';

        if (dprintf(STDOUT_FILENO, WORKER_MESSAGE_FORMAT, getpid(), hash, path) == -1)
        {
            perror("Worker write error");
            free(path);
            return RW_ERROR;
        }
    }
    int aux = errno;
    free(path);

    if (read_len == -1 && (aux == EINVAL || aux == ENOMEM))
    {
        perror("Worker read error");
        return RW_ERROR;
    }
    return SUCCESS;
}

