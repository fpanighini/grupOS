#ifndef LIB_H
#define LIB_H

#define MD5_LEN 32
#define PATH_MAX 4096
#define ARG_MAX 2097152
#define FILES_MAX ((ARG_MAX) / (PATH_MAX))

typedef struct Task
{
    char path[PATH_MAX];
} Task;

typedef struct Result
{
    char path[PATH_MAX];
    char hash[MD5_LEN];
} Result;

#endif
