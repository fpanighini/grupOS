#include "application.h"

void close_pipes(int pipe_fds[][2], size_t count)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        close_pipe(pipe_fds[i]);
    }
}

void close_files(FILE * files[], size_t count)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        fclose(files[i]);
    }
}

// char * create_mem(){
//     const char* shm_name = SHARED_MEM_NAME;
// 
//     int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
//     if (shm_fd == -1){
//         perror("shm_open");
//         return MEM_ERROR;
//     }
// 
//     if (ftruncate(shm_fd, SHM_SIZE) == -1){
//         perror("ftruncate");
//         return MEM_ERROR;
//     }
// 
//     char *shared_mem = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (shared_mem == MAP_FAILED){
//         perror("mmap");
//         return MEM_ERROR;
//     }
// 
//     return shared_mem;
// }


// int destroy_mem(char * shared_mem){
//     if(munmap(shared_mem, SHM_SIZE) == -1){
//         perror("munmap");
//         return -1;
//     }
//     if(shm_unlink(SHARED_MEM_NAME) == -1){
//         perror("shm_unlink");
//         return -1;
//     }
//     return 0;
// }


