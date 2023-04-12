#ifndef VIEWER_H
#define VIEWER_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "../../lib/lib.h"
#include <sys/select.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

char * create_mem();
int destroy_mem(char * shared_mem);

#endif // VIEWER_H
