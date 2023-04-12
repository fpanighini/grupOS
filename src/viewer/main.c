#include "viewer.h"
#include <unistd.h>

int main(int argc, char const *argv[])
{
    char read_buf[100];
    const char* shm_name;
    if (argc == 1){
        read(0, read_buf, 15);
        shm_name = read_buf;
    }
    else {
        shm_name = argv[1];
    }


    printf("%s\n", shm_name);

    const char* shm_buf_name = SHARED_MEM_BUF_NAME;

    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1){
        perror("shm_open 1");
        return -1;
    }

    int shm_buf_fd = shm_open(shm_buf_name, O_RDWR, 0666);
    if (shm_buf_fd == -1){
        perror("shm_open");
        return -1;
    }

    Shm_t shared_mem;

    shared_mem.size = (int *) mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem.size == MAP_FAILED){
        perror("mmap");
        return -1;
    }

     shared_mem.buf = (char *) mmap(NULL, SHM_WIDTH * *shared_mem.size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_buf_fd, 0);
     if (shared_mem.buf == MAP_FAILED){
         perror("mmap");
         return -1;
     }
    sem_t *sem = sem_open(SEMAPHORE_NAME_READ_BUFFER, O_RDWR);
    sem_t *sem_viewer = sem_open(SEMAPHORE_NAME_VIEWER, O_RDWR);

    sem_wait(sem_viewer);

    printf("%s\n", shared_mem.buf);
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


    if(munmap(shared_mem.buf, SHM_WIDTH * *shared_mem.size) == -1){
        perror("munmap viewer");
        return -1;
    }


    if(munmap(shared_mem.size, sizeof(int)) == -1){
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
