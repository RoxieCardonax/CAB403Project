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
//#include "hashTable.h"
#include "shared_mem.h"

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

#define SHARED_MEMORY "PARKING"
#define SHM_SIZE 2920 

#define START_COUNT 5
#define TEMP_COUNT 35

#define SMOOTH_TEMPS 5

//Other notes - Sensor has to read values every 2 milliseconds


// --------------------SHARED MEMORY--------------------
//Why do we need shared memory?
//Each process has its own address space, if any process wants to communicate with some information from its own address space to other processes, 
//then it is only possible with IPC techniques
shared_mem_t *shm; 

// initalize arrays for different temperature conditions 
int smooth_temps[LEVELS][TEMPCHANGE_WINDOW]; // Array of Temperatures for each level
int median_temp[LEVELS][MEDIAN_WINDOW]; // Array of Median Temperatures
int current_temp[LEVELS][MEDIAN_WINDOW]; // Array of Current Temperatures 

// --------------------MUTEX--------------------
// Mutex is a lock that we set before using a shared resource and release after using it
// When the lock is set, no other thread can access the locked region of code

//Create Mutexes for when fire is detected
pthread_mutex_t fire_mutex; //Mutex for fire (pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER)
pthread_cond_t fire_condition; //Mutex for condition (pthread_cond_t alarm_condvar = PTHREAD_COND_INITIALIZER;)
int alarm_active = 0; // variable for fire 0 = false, 1 = true

// --------------------SECTION: TEMP CALCS--------------------

#include <stdio.h>

#define MEDIAN_WINDOW 5
#define LEVELS 5

int curr_temp[LEVELS][MEDIAN_WINDOW];

void smooth_temp(int a[], int n);
void prn_array(char* s, int a[], int n);

//---- SWAP FUNCTION ----
void swap(int p, int q)
{
    int tmp;
    tmp = p;
    p = q;
    q = tmp;
}

//---- TEMP MONITOR FUNCTION ----
void* temp_monitor(void* ptr) {
	int thread = *((int*)ptr);
	int temperature;
	temperature = shared_mem_t->levels[thread].temp;

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
            temperature = shared_mem_t->levels[thread].temp;
            temp_list[i] = temperature;
        }

        //-- Evaluate temps for smoothing--
        for (count = START_COUNT; count < TEMP_COUNT; count++){
            temperature = shared_mem_t->levels[thread].temp;
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
                temperature = swap(temporary_list[min_i],temporary_list[i]);
            }
            //-- Find Median--
            median_temp = temporary_list[2];

            median_list[rep] = median_temp;
            rep++;

        }
    }
}


//---- MAIN ----
int main(){
    //--STILL WORKING ON--
    return 0;
}
