// Needs to handle queue's for entrances and exits
// Needs to handle the time that the car has been in there
// Simulat the time it takes for the boom gates to opperate

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include "resources/shared_mem.h"
#include "resources/queue.h"
#include "resources/generatePlate.h"
#include "resources/hashTable.h"
#include "resources/test.h"
#include <time.h>

int fd;
parking_data_t *shm; // Initilize Shared Memory Segment

// Mutex for queues
pthread_mutex_t queues_mutex;
pthread_cond_t queues_cond;

// Thread information struct
typedef struct thread_info
{

    queue *queues;

    parking_data_t *shm;

    int num;

} thread_info_t;

// Function Help from https://qnaplus.com/c-program-to-sleep-in-milliseconds/ Author: Srikanta
// Input microseconds
int threadSleep(long tms)
{
    usleep(tms * 1000);
}

// BoomGate Entrance Operations
void *runEntryBG(void *arg)
{
    int num = *(int *)arg; // ENTRANCE NUM

    for (;;)

    {
        pthread_mutex_lock(&shm->entrys[num].boomgate_mutex);
        while (shm->entrys[num].boomgate == 'C' || shm->entrys[num].boomgate == 'O')
        {
            pthread_cond_wait(&shm->entrys[num].boomgate_cond, &shm->entrys[num].boomgate_mutex);
        }
        // Boom Gate Rising.
        if (shm->entrys[num].boomgate == 'R')
        {

            printf("Boomgate at EntranceBG '%d' is Rising...\n", num + 1);
            printf("Status of Entrance BG: %d is %c \n\n ", num + 1, shm->entrys[num].boomgate);
            threadSleep(10); // Wait 10ms for gate to open

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
            threadSleep(10); // Wait 10ms for gate to close

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
        // Boom Gate Rising.
        while (shm->exits[num].boomgate == 'C' || shm->exits[num].boomgate == 'O')
        {
            pthread_cond_wait(&shm->exits[num].boomgate_cond, &shm->exits[num].boomgate_mutex);
        }
        if (shm->exits[num].boomgate == 'R')
        {

            printf("Boomgate at ExitBG '%d' is Rising...\n", num + 1);
            printf("Status of Exit BG: %d is %c \n\n ", num + 1, shm->exits[num].boomgate);
            threadSleep(1000); // Wait 10ms for gate to open

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
            threadSleep(1000); // Wait 10ms for gate to close

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

// Initilise Boomgate Threads
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
// Listen to boomgates and remove from queue
void *remove_from_entry_queue(void *args)
{

    // Listen to boom gate cond
    printf("\n\na threads\n\n");

    // Get info from args
    queue *queues = ((thread_info_t *)args)->queues;
    parking_data_t *shm = ((thread_info_t *)args)->shm;
    int num = ((thread_info_t *)args)->num;

    free(args);

    printf("ADDRESS OF PARKING %p\n", shm);

    // Lock mutex
    // pthread_mutex_lock(&shm->entrys[num].LPR_mutex);

    while (1)
    {

        // Wait for LPR condition, unlock mutex until ready
        pthread_cond_wait(&shm->entrys[num].LPR_cond, &shm->entrys[num].LPR_mutex);

        // temp sleep
        sleep(2);

        // Allocate memory for removed car
        char *removed = malloc(sizeof(char) * 6);

        // strcpy(removed, );

        printf("%s", removeFromQueue(&queues[num]));
        // free mem
        // free(removed);

        // Free conditional
        pthread_cond_signal(&shm->entrys[num].LPR_cond);
    }
}
// Spawn car thread
void *spawn_cars(void *args)
{

    // Create array of queues
    queue *q = malloc(Num_Of_Entries * sizeof(*q));

    // Create queue's needed and boomgate listeners
    for (int i = 0; i < Num_Of_Entries; i++)
    {

        // Init queue in array
        initQueue(&q[i]);

        // Allocate memory for variables in struct
        q[i].front = malloc(sizeof(item *));
        q[i].rear = malloc(sizeof(item *));
        q[i].count = *(int *)malloc(sizeof(int *));

        // Create struct to pass to each thread
        thread_info_t *ti = malloc(sizeof(thread_info_t));
        ti->queues = q;
        ti->shm = args;
        ti->num = i;

        // Init thread for listening to boom gates and removing
        // from front of queue
        pthread_t bg_listener_thread;

        // Create thread for listening to LPR_cond variables
        pthread_create(&bg_listener_thread, NULL, remove_from_entry_queue, ti);
    }

    parking_data_t *shm = args;

    // Loop constantly
    while (1)
    {
        // Need to generate new car at random queue every 1-100ms
        int interval = randomNumber();
        int counter = 0;
        // Sleep for millisecond (Keeps thread asleep majority of the time)
        sleep(interval / 1000);

        // Increase counter
        counter++;

        // Test to see if counter is at interval
        if (counter == interval)
        {
            // Get number plate to add
            char *numberPlate = generateNumberPlate();
            // Time is at interval, span new car
            addToQueue(&q[randomNumber() % 5], numberPlate);
            pthread_cond_broadcast(&shm->entrys[randomNumber() % 5].LPR_cond);
            counter = 0;
        }
    }
}

int main()
{

    parking_data_t parking; // Initilize parking segment

    // Create thread for adding cars to queues
    pthread_t spawn_car_thread;

    // Map Parking Segment to Memory and retrive address.
    shm = create_shared_memory(&parking);

    // USED TO TEST AND VALIDATE MEMORY SEGMENT
    // testSegment(shm);
    // Create thread
    pthread_create(&spawn_car_thread, NULL, spawn_cars, NULL);
    if (pthread_create(&spawn_car_thread, NULL, spawn_cars, NULL) != 0)
    {
        printf("Could not create thread");
    }

    // Create thread for each entry
    pthread_create(&spawn_car_thread, NULL, spawn_cars, shm);

    pthread_join(spawn_car_thread, NULL);

    // Create BoomGate Threads
    pthread_t *threads = malloc(sizeof(pthread_t) * (Num_Of_Entries + Num_Of_Exits));
    createBoomGateThreads(shm, threads);

    // Generate Random Temperature
    // Helper Function from https://stackoverflow.com/questions/29381843/generate-random-number-in-range-min-max
    srand(time(0));
    for (;;)
    {
        threadSleep(1 + rand() % (5 + 1 - 1)); // New Temperature 1-5ms.
        for (int i = 0; i < Num_Of_Level; i++)
        {
            int16_t temp = 18 + rand() % (50 + 1 - 18); // Calculate new temperature between 18C-50C.
            shm->levels[i].temp = temp;                 // Set new temp to each level
        }
    }

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