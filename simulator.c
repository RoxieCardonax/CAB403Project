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

int fd;

void testSegment(parking_data_t *shm)
{
    printf("SIZE OF  ENTRY VALUE %ld\n", sizeof(entrys_t));       // Check size of entrys
    printf("SIZE OF  EXIT VALUE %ld\n", sizeof(exits_t));         // Check size of exits
    printf("SIZE OF  LEVEL VALUE %ld\n", sizeof(levels_t));       // Check size of levels
    printf("SIZE OF  PARKING SEGMENT VALUE %ld\n", sizeof(*shm)); // Check size of memory segment

    /* //Test Directly Accessing Entrance Boomgate Value to address
    char *c = (char*)shm + 184;
    *c = 'c';
     printf("TEST%p\n",c);

    //est Using pointers to assign Entrance Boomgate Value to address
     shm->entrys[0].boomgate = 'A'; */

    // Check Structs are mapped correctly in memory
    printf("MEMORY ADDRESS OF SHM + 184: %p\n", (char *)(shm + 184)); // Memory address of Entrance 1 boomgate

    // Wait for Manager to change value
    // while (shm->exits[1].boomgate != 'C')
    // sleep;

    printf("SHM VALUE Returned %c\n", shm->entrys[1].boomgate); // Print Value
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
    if (ftruncate(fd, sizeof(parking_data_t)) == -1)
    {
        printf("Failed to set capacity of memory segment");
    };

    // Map memory segment to pysical address
    shm = mmap(NULL, sizeof(parking_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (shm == MAP_FAILED)
    {
        printf("FAILED TO MAP shared memory segment.\n");
    }
    printf("Created shared memory segment.\n");
    printf("ADDRESS OF PARKING %p\n", shm);

    return shm;
}

// Initialize queue for each entrance
queue *init_queues()
{
    // Init queue array
    queue *queues = malloc(Num_Of_Entries * sizeof(queue *));

    // Allocate memory and assign pointer to array
    for (int i = 0; i < Num_Of_Entries; i++)
    {
        // Create queue
        struct queue *newQueue = malloc(sizeof(queue));
        newQueue->count = *(int *)malloc(sizeof(int));
        newQueue->front = (item *)malloc(sizeof(item));
        newQueue->rear = (item *)malloc(sizeof(item));

        // Initialize queue
        initQueue(newQueue);

        // Add queue to array
        queues[i] = *newQueue;
    };

    // Test initialization
    if (queues[0].count != 0)
    {
        return NULL;
    };

    // Return pointer to start of array
    return queues;
}
// Add number plate to random queue after certain amount of time
void generateCarAtEntry(queue *queues)
{
    // Needs to be threaded/mutexed
    // TBD
    // Lock mutex
    pthread_mutex_lock(&queues_mutex);
    // Pick a queue
    int chosenQueue = randomNumber() % Num_Of_Entries;

    // Generate plate and add it to queue
    addToQueue((queue *)&queues[chosenQueue], generateNumberPlate());

    // Unlock mutex
    pthread_mutex_unlock(&queues_mutex);
}

// Entry testing
void *testEntry(void *arg)
{
    // Create array of queues
    queue *queues = init_queues();

    // Loop through and add number
    for (int i = 0; i < Num_Of_Entries; i++)
    {
        printf("%d\ncount\n", queues[i].count);
        generateCarAtEntry(queues);
        printf("%d\nshouldbemore\n", queues[i].count);
        showQueue(queues[i].front);
    }
    for (int i = 0; i < 5; i++)
    {
        printf("removbing");
        while (!isEmpty((queue *)&queues[i]))
        {
            removeFromQueue((queue *)&queues[i]);
        }
    }
    for (int i = 0; i < Num_Of_Entries; i++)
    {
        for (int j = 0; j < queues[i].count; i++)
        {
            printf("hello");
            printf("%s", removeFromQueue(&queues[i]));
        }
    }

    return NULL;
}
void *spawn_cars(void *arg)
{
    printf("\n\n\n%d\n\n\nhi");
    testEntry(arg);
    free(arg);
    return NULL;
}
int main()
{
    // Define Queue's
    queue *q1 = (queue *)malloc(sizeof(queue));
    queue *q2 = (queue *)malloc(sizeof(queue));
    queue *q3 = (queue *)malloc(sizeof(queue));
    queue *q4 = (queue *)malloc(sizeof(queue));
    queue *q5 = (queue *)malloc(sizeof(queue));

    queue queues[] = {*q1, *q2, *q3, *q4, *q5};

    parking_data_t parking; // Initilize parking segment
    parking_data_t *shm;    // Holds Mapped Shm Address

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

    // Do it again bc why not

    // testEntry();
    // spawn_cars();
    // pthread_join(spawn_car_thread, NULL);

    // Close shared memory
    if ((munmap(shm, SHMSZ)) == -1)
    {
        perror("munmap failed");
    }

    close(fd);

    return 0;
}