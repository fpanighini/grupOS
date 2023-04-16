#ifndef WORKER_H
#define WORKER_H

#include "../../lib/lib.h"
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define MD5SUM_PATH "/usr/bin/md5sum"

int md5sum(char *hash, char *const path);



#endif // WORKER_H
