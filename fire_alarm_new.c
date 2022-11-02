// --------------------LIBRARIES--------------------
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <inttypes.h>
#include <stdbool.h>

//#include "hashTable.h"
#include "resources/shm.h"
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

int fd;
int levels_num = 5;
int alarm_active = false; 

// --------------------SHARED MEMORY--------------------
//Why do we need shared memory?
//Each process has its own address space, if any process wants to communicate with some information from its own address space to other processes, 
//then it is only possible with IPC techniques
parking_data_t *shm; 


// --------------------FUNCTIONS--------------------

//---- TEMP MONITOR FUNCTION ----
void* temp_monitor(void* ptr) {
	int thread = *((int*)ptr);
	int temperature;
	temperature = shm->levels[thread].temp;

    // -- Monitor Temps (While loop)--
    while(temperature !=0){

        //Initialise lists
        int temp_list[35];
        int median_list [30];

        //Initialise other variables
        int count = 0; 
        int median_temp;
        int fixed_temp_count;
        int rep; 

        //-- Evaluate First 5 Temps and store in list--
        for (int i = 0; i<LEVELS; i++){
            temperature = shm->levels[thread].temp;
            temp_list[i] = temperature;
        }

        //-- Evaluate temps for smoothing--
        for (count = START_COUNT; count < TEMP_COUNT; count++){
            temperature = shm->levels[thread].temp;
            temp_list[count] = temperature;

            int temporary_list[SMOOTH_TEMPS];
            for (int i =0; i < SMOOTH_TEMPS; i++){
                temporary_list[i] = temporary_list[count - SMOOTH_TEMPS+i];
            }

            //-- Sort temps -- from lecture
			int n = sizeof(temporary_list) / sizeof(temporary_list[0]);
            for (int i = 0; i < n-1; i++){
                // Find minimum
                int min_i = i;

                for (int j = i+1; j < n; j++){
                    if (temporary_list[j]< temporary_list[min_i]){
                        min_i = j;
                    }
                }
                //-- Swap min elemtent with first to sort--
				int temp = temp_list[min_i];
				temporary_list[min_i] = temporary_list[i];
				temporary_list[i] = temp;
            }
            //-- Find Median--
            median_temp = temporary_list[2];

            median_list[rep] = median_temp;
            rep++;
        }
        // Fire detection portion of code
        fixed_temp_count = 0;
        //-- Fire detection for smoothed temps over threshold
        for (int i =0; i < TEMPCHANGE_WINDOW; i++){
            if (median_list[i] >= FIXED_TEMP){
                fixed_temp_count++;
            }
        }
        if (fixed_temp_count >= TEMPCHANGE_WINDOW*0.9){
            alarm_active = 1; //TRUE - FIRE IS DETECTED
        }
        //Fire detection by temperature rise
        if (median_list[30] - median_list[0] > 8){
            alarm_active = 1; //TRUE - FIRE IS DETECTED
        }

        usleep(2000);
    }
    return NULL;
}


//---- THREADS ----
// -- Levels thread--
void init_level_thread(){
    pthread_t levels_threads [LEVELS];
    int level[LEVELS];

    for (int i = 0; i < LEVELS; i++){
		level[i] = i;
		pthread_create(&levels_threads[i], NULL, temp_monitor, &level[i]);    
    }
    printf("Created levels thread.\n");

}

// SHARED MEMORY
// Create Shared Memory segment on startup.
void *create_shmory(parking_data_t *shm)
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

//---- MAIN ----
int main()
{   
    //Initialise Shared Memory
    parking_data_t parking; // Initilize parking segment
    
    // Map Parking Segment to Memory and retrive address.
    shm = create_shmory(&parking);

	// Initialise level thread
    init_level_thread();

    // Fire Process - in while loop
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

}
