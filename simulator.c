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

// BoomGate Entrance Operations
void *runEntryBG(void *arg)
{
    int num = *(int *)arg; // ENTRANCE NUM

    for (;;)

    {
        pthread_mutex_lock(&shm->entrys[num].boomgate_mutex);
        pthread_cond_wait(&shm->entrys[num].boomgate_cond, &shm->entrys[num].boomgate_mutex);

        // Boom Gate Rising.
        if (shm->entrys[num].boomgate == 'R')
        {

            printf("Boomgate at EntranceBG '%d' is Rising...\n", num + 1);
            printf("Status of Entrance BG: %d is %c \n\n ", num + 1, shm->entrys[num].boomgate);
            sleep(2); // Change to 10ms to open gate

            // SET BOOMGATE TO OPEN
            shm->entrys[num].boomgate = 'O';
            printf("Boomgate at EntranceBG '%d' is Opened\n", num + 1);
            printf("Status of Entrance BG: %d is %c \n\n ", num + 1, shm->entrys[num].boomgate);
            pthread_cond_broadcast(&shm->entrys[num].boomgate_cond);
            pthread_mutex_unlock(&shm->entrys[num].boomgate_mutex);
        }

        // Boom Gate Lowering
        if (shm->entrys[num].boomgate == 'L')
        {
            printf("Boomgate at EntranceBG '%d' is Lowering...\n", num + 1);
            printf("Status of Entrance BG: %d is %c \n\n ", num + 1, shm->entrys[num].boomgate);
            sleep(2); // Change to 10ms to open and close gate

            // SET BOOMGATE TO CLOSED
            shm->entrys[num].boomgate = 'C';
            printf("Boomgate at EntranceBG '%d' is Closed...\n", num + 1);
            printf("Status of Entrance BG: %d is %c \n\n ", num + 1, shm->entrys[num].boomgate);
            pthread_cond_broadcast(&shm->entrys[num].boomgate_cond);
            pthread_mutex_unlock(&shm->entrys[num].boomgate_mutex);
        }
    }

    free(arg);
}

// BoomGate Exit Operations
void *runExitBG(void *arg)
{
    int num = *(int *)arg; // EXIT NUM

    for (;;)

    {
        pthread_mutex_lock(&shm->exits[num].boomgate_mutex);
        pthread_cond_wait(&shm->exits[num].boomgate_cond, &shm->exits[num].boomgate_mutex);

        // Boom Gate Rising.
        if (shm->exits[num].boomgate == 'R')
        {

            printf("Boomgate at ExitBG '%d' is Rising...\n", num + 1);
            printf("Status of Exit BG: %d is %c \n\n ", num + 1, shm->exits[num].boomgate);
            sleep(2); // Change to 10ms to open gate

            // SET BOOMGATE TO OPEN
            shm->exits[num].boomgate = 'O';
            printf("Boomgate at ExitBG '%d' is Opened\n", num + 1);
            printf("Status of ExitBG: %d is %c \n\n ", num + 1, shm->exits[num].boomgate);
            pthread_cond_broadcast(&shm->exits[num].boomgate_cond);
            pthread_mutex_unlock(&shm->exits[num].boomgate_mutex);
        }

        // Boom Gate Lowering
        if (shm->exits[num].boomgate == 'L')
        {
            printf("Boomgate at ExitBG '%d' is Lowering...\n", num + 1);
            printf("Status of ExitBG BG: %d is %c \n\n ", num + 1, shm->exits[num].boomgate);
            sleep(2); // Change to 10ms to open and close gate

            // SET BOOMGATE TO CLOSED
            shm->exits[num].boomgate = 'C';
            printf("Boomgate at ExitBG '%d' is Closed...\n", num + 1);
            printf("Status of ExitBG: %d is %c \n\n ", num + 1, shm->exits[num].boomgate);
            pthread_cond_broadcast(&shm->exits[num].boomgate_cond);
            pthread_mutex_unlock(&shm->exits[num].boomgate_mutex);
        }
    }

    free(arg);
}


//Initilise Boomgate Threads
void createBoomGateThreads(parking_data_t *shm, pthread_t *threads)
{
    for (int i = 0; i < Num_Of_Entries; i++)
    {
        int *entNum = malloc(sizeof(int));
        *entNum = i;
        if (pthread_create(threads + i, NULL, runEntryBG, entNum) != 0)
        {
            printf("Error, could not create thread for boomgate.");
        };
    }

    for (int i = 0; i < Num_Of_Exits; i++)
    {
        int *extNum = malloc(sizeof(int));
        *extNum = i;
        if (pthread_create(threads + Num_Of_Entries + i, NULL, runExitBG, extNum) != 0)
        {
            printf("Error, could not create thread for boomgate.");
        };
    }
}

// Create Shared Memory segment on startup.
void *create_shared_memory(parking_data_t *shm)
{

    // Check for previous memory segment.Remove if exits
    shm_unlink(SHARE_NAME);

    // Using share name and both creating and setting to read and write. Read and write for owner, group (0666). Fail if negative int is returned
    int open;
    open = shm_open(SHARE_NAME, O_CREAT | O_RDWR, 0666);

    if (open < 0)
    {
        printf("Failed to create memory segment");
    }
    fd = open;

    // Configure the size of the memory segment to 2920 bytes
    if (ftruncate(fd, SHMSZ) == -1)
    {
        printf("Failed to set capacity of memory segment");
    };

    // Map memory segment to pysical address
    shm = mmap(NULL, SHMSZ, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm == MAP_FAILED)
    {
        printf("FAILED TO MAP shared memory segment.\n");
    }
    printf("Created shared memory segment.\n");
    printf("ADDRESS OF PARKING %p\n", shm);

    return shm;
}

int main()
{

    parking_data_t parking; // Initilize parking segment

    // Map Parking Segment to Memory and retrive address.
    shm = create_shared_memory(&parking);

    // USED TO TEST AND VALIDATE MEMORY SEGMENT
    // testSegment(shm);

    // Initialise Mutex/Condition Variables and Set Default Values for Shared Memory
    setDefaultValues(shm);

    // Create BoomGate Threads
    pthread_t *threads = malloc(sizeof(pthread_t) * (Num_Of_Entries + Num_Of_Exits));
    createBoomGateThreads(shm, threads);

    // Clean Up Threads and Shared Memory Mapping
    for (int i = 0; i < Num_Of_Entries + Num_Of_Exits; i++)
    {
        pthread_join(threads[i], NULL);
    }

    if ((munmap(shm, SHMSZ)) == -1)
    {
        perror("munmap failed");
    }

    close(fd);
    return 0;
}