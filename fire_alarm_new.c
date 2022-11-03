// --------------------LIBRARIES--------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <inttypes.h>
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
// --------------------DEFINITIONS--------------------
// Carpark format
#define LEVELS  5   //Given from task - how many carpark levels there are
#define ENTRANCES   5   //Given from task - how many entrances there are
#define EXITS   5   //Given from task - how many exits there are

// Temperature Information
#define MEDIAN_WINDOW 5 //Given from task - 5 most recent temp readings - median temperature will be recorded as smoothes
#define TEMPCHANGE_WINDOW 30 //Given from task - 30 most recent smoothed temperatures produced by sensor - if 90% of these are 58 degrees _ the temperature is considered high enough that there must be a fire

#define FIXED_TEMP 58 // Given from task - Average temperature max limit
#define RISE_TEMP 8 //Given from task - If temperature is 8+ hotter than the 30 most recent temperatures then the temperature is considered to be growing at a fast enough rate that there must be a fire

#define SHARE_NAME "PARKING"
#define SHM_SIZE 2920 

#define START_COUNT 5
#define TEMP_COUNT 35

#define SMOOTH_TEMPS 5

bool alarm_active = false;
parking_data_t* shm;
int fd;

void* temp_monitor(void* ptr) {
	int thread = *((int*)ptr);
	int temperature;
	temperature = shm->levels[thread].temp;

	/* Monitor Temperatures */
	while(temperature != 0) {

		int tempList[35];
		int medianList[30];
		int count = 0;
		int medianTemp;
		int fixedTempCount;
		int iterations = 0;

		/* Evaluate the first five temperatures before smoothing*/
		for (int i = 0; i < LEVELS; i++) {
			temperature = shm->levels[thread].temp;
			tempList[i] = temperature;
		}

		/* Evaluate temps 5-35 for smoothing*/
		for (count = 5; count < 35; count++) {
			temperature = shm->levels[thread].temp;
			tempList[count] = temperature;

			int tempor_list[5];
			for (int i = 0; i < 5; i++) {
				tempor_list[i] = tempList[count - 5 + i];
			}

			/* Sort temp list using selection sort */
			int n = sizeof(tempor_list) / sizeof(tempor_list[0]);
			int ix, jx, min_ix;

			for (ix = 0; ix < n - 1; ix++)
			{
				// Find the minimum element in unsorted array
				min_ix = ix;
				for (jx = ix + 1; jx < n; jx++)
					if (tempor_list[jx] < tempor_list[min_ix])
						min_ix = jx;

				// Swap the found minimum element with the first element
				int temperature = tempList[min_ix];
				tempor_list[min_ix] = tempor_list[ix];
				tempor_list[ix] = temperature;
			}

			/* Find median */
			medianTemp = tempor_list[2];
			

			medianList[iterations] = medianTemp;
			iterations++;
		}
		
		/* Calculate fixed temperature fire detection */
		fixedTempCount = 0;
		for (int i = 0; i < 30; i++) {
			//printf("median temp: %d \n", medianList[i]);
			if (medianList[i] >= 58) {
				fixedTempCount++;
			}
		}

		if (fixedTempCount >= 27) {
			alarm_active = true;
		}

		/* Calculate rate of rise fire detection */
		if ((medianList[30] - medianList[0]) > 8) {
			alarm_active = true;
		}

		usleep(2000);
	}
	return NULL;
}

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
    return shm;
}

int main()
{
    //Initialise Shared Memory
    parking_data_t parking; // Initilize parking segment
    
    // Map Parking Segment to Memory and retrive address.
    shm = create_shmory(&parking);

	/* Create a thread for each level */
	pthread_t threads[LEVELS];

	int level[LEVELS];
	for (int i = 0; i < LEVELS; i++) {

		level[i] = i;
		pthread_create(&threads[i], NULL, temp_monitor, &level[i]);
	}

	while(shm->levels[0].temp >= 0) {
		/* Activate Alarm */
		if (alarm_active) {
			fprintf(stderr, "*** ALARM ACTIVE ***\n");

			/* Handle the alarm system and open boom gates
			   Activate alarms on all levels */
			for (int i = 0; i < LEVELS; i++) {
				shm->levels[i].alarm = true;
			}

			/* Show evacuation message */
			char* evacmessage = "EVACUATE ";
			for (char* p = evacmessage; *p != '\0'; p++) {
				for (int i = 0; i < LEVELS; i++) {
					pthread_mutex_lock(&shm->entrys[i].info_mutex);

					shm->entrys[i].display = *p;
					pthread_cond_signal(&shm->entrys[i].info_cond);

					pthread_mutex_unlock(&shm->entrys[i].info_mutex);
				}
				usleep(20000);
			}
			alarm_active = false;
		}
		else {
			for (int i = 0; i < LEVELS; ++i) {
				shm->levels[i].alarm = false;
			}
		}
		usleep(1000);
	}

	munmap(shm, SHM_SIZE);
	close(fd);
}
