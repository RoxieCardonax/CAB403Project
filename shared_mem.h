
#ifndef SHARED_MEM
#define SHARED_MEM



#include <sys/types.h>
#include <pthread.h>


#define SHARE_NAME "PARKING" //Shared Memory Name
#define SHMSZ 2920 //Size of Shared Memory to be allocated
#define Num_Of_Entries 5 // Number of entriess
#define Num_Of_Exits 5 // Number of Exits
#define Num_Of_Level 5 //Num of Car Park Levels
#define Max_Per_Level 20 //Number of Max Car Parks



/**
 * Structs Types for Entries, Exits and Levels Pointers
 */
typedef struct entrys {
	pthread_mutex_t *LPR_mutex;
	pthread_mutex_t *boomgate_mutex;
	pthread_mutex_t *info_mutex;
	pthread_cond_t *LPR_cond;
	pthread_cond_t *boomgate_cond;
	pthread_cond_t *info_cond;
	char *lpr[6];
	char *boomgate;
	char *display;
}entrys_t ;


typedef struct exits {
	pthread_mutex_t *LPR_mutex;
	pthread_mutex_t *boomgate_mutex;
	pthread_cond_t *LPR_cond;
	pthread_cond_t *boomgate_cond;
	char *lpr[6];
	char *boomgate;
} exits_t;

typedef struct levels {
	pthread_mutex_t *LPR_mutex;
	pthread_cond_t *LPR_cond;
	char *lpr[6];
	char *alarm;
	int16_t *temp;
}levels_t ;


/**
 * Parking Data Block
 */
typedef struct shared_data {

    //Entry Struct
    entrys_t entrys[Num_Of_Entries];

    //Exit Struct
    exits_t exits[Num_Of_Exits];

    //Level Struct
    levels_t levels[Num_Of_Level];
	
	int fd;
	char *test;
} parking_data_t;


#endif
