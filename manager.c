#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include "resources/shared_mem.h"
#include "resources/generatePlate.h"
#include "resources/hashTable.h"
#include <stdio.h>
#include "resources/test.h"

int fd;
parking_data_t *shm; // Initilize Shared Memory Segment



//TESTING BOOMGATE COMMUNICATION WITH SIMULATOR ON EXIT 1
//CHANGE THE INDEX TO WHATEVER NUMBER YOU WANT TO TEST
void testBoomgate()

{
    // TEST BOOMGATE OPENING
    printf("RAISING BOOMGATE...\n");
    pthread_mutex_lock(&shm->exits[0].boomgate_mutex);
    shm->exits[0].boomgate = 'R';
    pthread_cond_broadcast(&shm->exits[0].boomgate_cond);
    pthread_mutex_unlock(&shm->exits[0].boomgate_mutex);
    printf("Current Status of BOOMGATE %c\n\n", shm->exits[0].boomgate);

    // CAR CAN ONLY DRIVE IN WHEN GATE IS OPEN
    printf("WAITING ON BOOMGATE TO BE OPENED...\n");
    pthread_cond_wait(&shm->exits[0].boomgate_cond, &shm->exits[0].boomgate_mutex);
    if (shm->exits[0].boomgate == 'O')
    {
        printf("CAR DRIVING IN\n\n");
        sleep(5);
    }
    pthread_mutex_unlock(&shm->exits[0].boomgate_mutex);

    // TEST BOOMGATE CLOSING
    printf("LOWERING BOOMGATE...\n");
    pthread_mutex_lock(&shm->exits[0].boomgate_mutex);
    shm->exits[0].boomgate = 'L';
    pthread_cond_broadcast(&shm->exits[0].boomgate_cond);
    pthread_mutex_unlock(&shm->exits[0].boomgate_mutex);

    sleep(2);
    printf("BOOMGATE IS CLOSED\n");
    sleep(1);
    printf("Current Status of BOOMGATE %c\n", shm->exits[0].boomgate);
}


// Read Shared Memory segment on startup.
void *read_shared_memory(parking_data_t *shm)
{

    // Using share name, return already created Shared Memory Segment.
    int open;
    open = shm_open(SHARE_NAME, O_RDWR, 0666);

    if (open < 0)
    {
        printf("Failed to create memory segment");
    }
    fd = open;

    // Map memory segment to physical address
    shm = mmap(NULL, SHMSZ, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm == MAP_FAILED)
    {
        printf("FAILED TO MAP shared memory segment.\n");
    }
    printf("Created shared memory segment.\n");
    printf("ADDRESS OF PARKING %p\n\n", shm);

    return shm;
}

int main()
{

    parking_data_t parking; // Initilize parking segment

    // Map Parking Segment to Memory and retrive address.
    shm = read_shared_memory(&parking);

    // USED TO TEST AND VALIDATE MEMORY SEGMENT
    // testSegment(shm);
    
    //USED TO TEST BOOMGATES
    //testBoomgate();

    if ((munmap(shm, SHMSZ)) == -1)
    {
        perror("munmap failed");
    }

    close(fd);
    return 0;
}