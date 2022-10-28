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

// Function Help from https://qnaplus.com/c-program-to-sleep-in-milliseconds/ Author: Srikanta
// Input microseconds
int threadSleep(long tms)
{
    usleep(tms * 1000);
}

/// TESTING BOOMGATE COMMUNICATION WITH SIMULATOR ON EXIT 1
// CHANGE THE INDEX TO WHATEVER NUMBER YOU WANT TO TEST
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
    while (shm->exits[0].boomgate == 'R')
    {
        pthread_cond_wait(&shm->exits[0].boomgate_cond, &shm->exits[0].boomgate_mutex);
    }
    if (shm->exits[0].boomgate == 'O')
    {
        printf("CAR DRIVING IN\n\n");
        threadSleep(1000); // Ten Milliseconds for car to drive in
    }
    // TEST BOOMGATE CLOSING
    printf("LOWERING BOOMGATE...\n");    
    shm->exits[0].boomgate = 'L';
    pthread_cond_broadcast(&shm->exits[0].boomgate_cond);
    pthread_mutex_unlock(&shm->exits[0].boomgate_mutex);
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

     // USED TO DISPLAY STATUS
     for (;;)
     {

         // Display Entrances
         printf("----------------ENTRANCES--------------\n");
         printf("LEVEL 1 LPR: %c     |     LEVEL 1 BG: %c\n", shm->entrys[0].lpr, shm->entrys[0].boomgate);
         printf("LEVEL 2 LPR: %c     |     LEVEL 2 BG: %c\n", shm->entrys[1].lpr, shm->entrys[1].boomgate);
         printf("LEVEL 3 LPR: %c     |     LEVEL 3 BG: %c\n", shm->entrys[2].lpr, shm->entrys[2].boomgate);
         printf("LEVEL 4 LPR: %c     |     LEVEL 4 BG: %c\n", shm->entrys[3].lpr, shm->entrys[3].boomgate);
         printf("LEVEL 5 LPR: %c     |     LEVEL 5 BG: %c\n", shm->entrys[4].lpr, shm->entrys[4].boomgate);
         printf("---------------------------------------\n");

         // Display Exits
         printf("------------------EXITS----------------\n");
         printf("LEVEL 1 LPR: %c     |     LEVEL 1 BG: %c\n", shm->exits[0].lpr, shm->exits[0].boomgate);
         printf("LEVEL 2 LPR: %c     |     LEVEL 2 BG: %c\n", shm->exits[1].lpr, shm->exits[1].boomgate);
         printf("LEVEL 3 LPR: %c     |     LEVEL 3 BG: %c\n", shm->exits[2].lpr, shm->exits[2].boomgate);
         printf("LEVEL 4 LPR: %c     |     LEVEL 4 BG: %c\n", shm->exits[3].lpr, shm->exits[3].boomgate);
         printf("LEVEL 5 LPR: %c     |     LEVEL 5 BG: %c\n", shm->exits[4].lpr, shm->exits[4].boomgate);
         printf("---------------------------------------\n");

         // Display Temperature
         printf("              ----TEMP---\n");
         printf("              LEVEL 1 : %dC\n", shm->levels[0].temp);
         printf("              LEVEL 2 : %dC\n", shm->levels[1].temp);
         printf("              LEVEL 3 : %dC\n", shm->levels[2].temp);
         printf("              LEVEL 4 : %dC\n", shm->levels[3].temp);
         printf("              LEVEL 5 : %dC\n", shm->levels[4].temp);
         printf("---------------------------------------\n");

         threadSleep(50); // Updates Every 'x' amount of milliseconds
         system("clear");
     } 

    // USED TO TEST BOOMGATES. RUN THIS WITH SIM RUNNING ALREADY
     testBoomgate();

    if ((munmap(shm, SHMSZ)) == -1)
    {
        perror("munmap failed");
    }

    close(fd);
    return 0;
}