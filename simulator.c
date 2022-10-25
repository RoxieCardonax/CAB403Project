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

void *runEntryBG(void *arg)
{
    int num = *(int *)arg;
    // Wait on Boomgate Value to open
    // Get Mutex lock for boomgate number
    // Wait for conidtion of boomgate
    // Change Value of boomgate status
    // It then unlocks the mutex
    // Broadcast to other mutexs on other threads waiting on thie mustx
    //
    printf("Locking Mutex at: EntranceBG '%d'\n", num + 1);
    // pthread_mutex_lock(&shm->entrys[num].boomgate_mutex);
    printf("Status at Entrance BG '%d': '%c'\n\n", num + 1, shm->entrys[num].boomgate);
    // pthread_mutex_unlock(&shm->entrys[num].boomgate_mutex);
    free(arg);
}

void *runExitBG(void *arg)
{
    int num = *(int *)arg;
    // Wait on Boomgate Value to open
    // Get Mutex lock for boomgate number
    // Wait for conidtion of boomgate
    // Change Value of boomgate status
    // It then unlocks the mutex
    // Broadcast to other mutexs on other threads waiting on thie mustx
    //
    printf("Locking Mutex at: ExitBG '%d'\n", num + 1);
    // pthread_mutex_lock(&shm->entrys[num].boomgate_mutex);
    printf("Status at Exit BG '%d': '%c'\n\n", num + 1, shm->exits[num].boomgate);

    free(arg);
}

void setDefaultValues(parking_data_t *shm)
{
    // Set Up Mutex/Condition Variables

    pthread_mutexattr_t mta;
    pthread_condattr_t cta;
    pthread_mutexattr_setpshared(&mta, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cta, PTHREAD_PROCESS_SHARED);

    // Entries
    for (int i = 0; i < Num_Of_Entries; i++)
    {
        // BoomGate
        pthread_mutex_init(&shm->entrys[i].boomgate_mutex, &mta);
        pthread_cond_init(&shm->entrys[i].boomgate_cond, &cta);
        shm->entrys[i].boomgate = 'C'; // Set deault gate to closed.

        // LPR
        pthread_mutex_init(&shm->entrys[i].LPR_mutex, &mta);
        pthread_cond_init(&shm->entrys[i].LPR_cond, &cta);

        // Information Sign
        pthread_mutex_init(&shm->entrys[i].info_mutex, &mta);
        pthread_cond_init(&shm->entrys[i].info_cond, &cta);
    }

    // Exits
    for (int i = 0; i < Num_Of_Exits; i++)
    {
        // BoomGate
        pthread_mutex_init(&shm->exits[i].boomgate_mutex, &mta);
        pthread_cond_init(&shm->exits[i].boomgate_cond, &cta);
        shm->exits[i].boomgate = 'C'; // Set deault gate to closed.

        // LPR
        pthread_mutex_init(&shm->exits[i].LPR_mutex, &mta);
        pthread_cond_init(&shm->exits[i].LPR_cond, &cta);
    }

    // Level
    for (int i = 0; i < Num_Of_Level; i++)
    {
        // LPR
        pthread_mutex_init(&shm->levels[i].LPR_mutex, &mta);
        pthread_cond_init(&shm->levels[i].LPR_cond, &cta);
    }
    printf("All mutexes created.\n");
}

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