#ifndef WORKER_H
#define WORKER_H

#include "../../lib/lib.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MD5SUM_PATH "md5sum"

#define SUCCESS 0
#define RW_ERROR 1

int md5sum(char *hash, char *const path);



#endif // WORKER_H
