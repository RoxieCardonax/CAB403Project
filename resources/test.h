#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include "shared_mem.h"
#include "generatePlate.h"
#include "hashTable.h"
#include <stdio.h>

///////Test Entrance/Exit Boomgate threads/////////////////
void *testEntranceBG(void *arg)
{

    int num = *(int *)arg;
    printf("BOOMGATE THREAD ON ENTRANCE LEVEL:'%d' created.\n", num + 1);
    free(arg);
}

void *testExitBG(void *arg)
{

    int num = *(int *)arg;
    printf("BOOMGATE TREAD ON EXIT LEVEL:'%d' created.\n", num + 1);
    free(arg);
}

///////Test Shared Memory Segment/////////////////
void testSegment(parking_data_t *shm)
{
    printf("SIZE OF  ENTRY VALUE %ld\n", sizeof(entrys_t));       // Check size of entrys
    printf("SIZE OF  EXIT VALUE %ld\n", sizeof(exits_t));         // Check size of exits
    printf("SIZE OF  LEVEL VALUE %ld\n", sizeof(levels_t));       // Check size of levels
    printf("SIZE OF  PARKING SEGMENT VALUE %ld\n", sizeof(*shm)); // Check size of memory segment

    // Test Directly Accessing Entrance Boomgate Value to address
    char *c = (char *)shm + 184;
    *c = 'c';
    printf("TEST DIRECT ACCESS %c\n", *c);

    // Test Using pointers to assign Entrance Boomgate Value to address
    shm->entrys[0].boomgate = 'A';
    printf("TEST POINTER ACCESS Returned %c\n", shm->entrys[0].boomgate); // Print Value

    // Check Structs are mapped correctly in memory
    printf("MEMORY ADDRESS OF SHM + 184: %p\n", (char *)(shm + 184)); // Memory address of Entrance 1 boomgate

    // Wait for Manager to change value
    // while (shm->exits[1].boomgate != 'C')
    // sleep;

    printf("Returned from manager changing %c\n\n", shm->entrys[1].boomgate); // Print Value
}
