//Memory 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include "shared_mem.h"


parking_data_t createPointers(parking_data_t* shm)

{    

parking_data_t *s; //ptr to address of element


    //Entry Setup
    for (int i=0;i<Num_Of_Entries;i++)
    {
        s=shm+288*i;			//Entry LPR Mutex
        
        shm->entrys[i].LPR_mutex=(pthread_mutex_t*)s;

        s=shm+40+288*i;			//Entry LPR Condition Variable
        shm->entrys[i].LPR_cond=(pthread_cond_t*)s;	        

        s=shm+(88+288*i);		//Entry LPR Values 
        for(int z=0;z<6;z++){
            shm->entrys[i].lpr[z]= (char*)s;
            s++;
        }

        s=shm+96+288*i;								//Entry Boomgate Mutex
        shm->entrys[i].boomgate_mutex=(pthread_mutex_t*)s;

        s=shm+136+288*i;							//Entry Boomgate Condition Variable
        shm->entrys[i].boomgate_cond=(pthread_cond_t*)s;

        s=shm+184+288*i;							//Entry Boomgate status
        shm->entrys[i].boomgate=(char*)s;

        s=shm+192+288*i;							//Entry Display Mutex
        shm->entrys[i].info_mutex=(pthread_mutex_t*)s;

        s=shm+232+288*i;							//Entry Display Condition Variable
        shm->entrys[i].info_cond=(pthread_cond_t*)s;

        s=shm+280+288*i;   							//Entry Display status
        shm->entrys[i].display=(char*)s;
       
        printf("ENTRANCE %d MAPPED\n",i);   
    }

     //Exit Setup
    for (int i=0;i<Num_Of_Exits;i++)
    {
        
        s=shm+1440+192*i;							//Exit LPR Mutex
        shm->exits[i].LPR_mutex=(pthread_mutex_t*)s;

        s=shm+1440+40+192*i;						//Exit LPR Condition Variable
        shm->exits[i].LPR_cond=(pthread_cond_t*)s;

        s=shm+1440+88+192*i;						//Exit LPR Values
        for(int z=0;z<6;z++){
            shm->exits[i].lpr[z]=(char*)s;
            s++;
        }

        s=shm+1440+96+192*i;						//Exit Boomgate Mutex
        shm->exits[i].boomgate_mutex=(pthread_mutex_t*)s;

        s=shm+1440+136+192*i;						//Exit Boomgate Condition Variable
        shm->exits[i].boomgate_cond=(pthread_cond_t*)s;

        s=shm+1440+184+192*i;						//Exit Boomgate status
        shm->exits[i].boomgate=(char*)s;	

         printf("EXIT %d MAPPED\n",i);   
    }  

    //Level Setup
    for (int i=0;i<Num_Of_Level;i++)
    {
        
        s=shm+2400+104*i;							//Level LPR Mutex
        shm->levels[i].LPR_mutex=(pthread_mutex_t*)s;

        s=shm+2400+40+104*i;						//Level LPR Condition Variable
        shm->levels[i].LPR_cond=(pthread_cond_t*)s;

        s=shm+2400+88+104*i;						//Level LPR values
        for(int z=0;z<6;z++)
        {
            shm->levels[i].lpr[z]=(char*)s;
            s++;
        }

        s=shm+2400+96+104*i;						//Level Temp Values
        shm->levels[i].temp=(int16_t*)s;

        s=shm+2400+98+104*i;						//Level Alarm value
        shm->levels[i].alarm=(char*)s;;	

        printf("LEVEL %d MAPPED\n",i);  
    }       
}

