#include "viewer.h"
#include <unistd.h>

int main(int argc, char const *argv[])
{
    const char* shm_name = SHARED_MEM_NAME;

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1){
        perror("shm_open");
        return -1;
    }

    char *shared_mem = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED){
        perror("mmap");
        return -1;
    }

    sem_t *sem = sem_open(SEMAPHORE_NAME_READ_BUFFER, O_RDWR);
    sem_t *sem_viewer = sem_open(SEMAPHORE_NAME_VIEWER, O_RDWR);

    sem_wait(sem_viewer);
    // TEST
    //char buf[100];

    // printf("start\n");

    
//     char *buffer = NULL;
//     size_t n = 0;

    // int i;
    // for (i = 0 ; i < 9 ; i++){
    //     sem_wait(sem);
    //     printf("while\n");
    //     read(shm_fd, &buf,99);
    //     printf("%s\n", buf);
    // }


    if(munmap(shared_mem, SHM_SIZE) == -1){
        perror("munmap viewer");
        return -1;
    }

    sem_post(sem_viewer);
    sem_close(sem);
    sem_close(sem_viewer);


    //printf("VIEWER\n");
    return 0;
}



// char * create_mem(){
//     const char* shm_name = SHARED_MEM_NAME;
// 
//     int shm_fd = shm_open(shm_name, O_RDWR, 0666);
//     if (shm_fd == -1){
//         perror("shm_open");
//         return MEM_ERROR;
//     }
// 
//     char *shared_mem = (char *) mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (shared_mem == MAP_FAILED){
//         perror("mmap");
//         return MEM_ERROR;
//     }
//     return shared_mem;
// }


// int destroy_mem(char * shared_mem){
//     if(munmap(shared_mem, SHM_SIZE) == -1){
//         perror("munmap viewer");
//         return -1;
//     }
//     return 0;
// }
