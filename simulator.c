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

int main()
{

    parking_data_t parking; // Initilize parking segment
    parking_data_t *shm;    // Holds Mapped Shm Address

    // Map Parking Segment to Memory and retrive address.
    shm = create_shared_memory(&parking);

    // USED TO TEST AND VALIDATE MEMORY SEGMENT
    // testSegment(shm);

    if ((munmap(shm, SHMSZ)) == -1)
    {
        perror("munmap failed");
    }

    close(fd);
    return 0;
}