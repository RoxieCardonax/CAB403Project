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
#include "memory_layout.h"




//Create Shared Memory segment on startup.
void* create_shared_memory (parking_data_t *shm)
{        
    //Check for previous memory segment.Remove if exits
    shm_unlink(SHARE_NAME);    

    //Using share name and both creating and setting to read and write. Read and write for owner, group (0666)
    //Fail if negative int is returned
    //Save FD to shared memory segement.
    int open;
    open = shm_open(SHARE_NAME,O_CREAT | O_RDWR, 0666);

    if (open <0)
    {
        printf("Failed to create memory segment");
        
    }
    shm->fd = open;

    //Configure the size of the memory segment to 2920 bytes
    if (ftruncate(shm->fd,SHMSZ) == -1)
    {        
        printf("Failed to set capacity of memory segment");
        
    };
    
     //Map memory segment to pysical address 
    shm = mmap(0,SHMSZ, PROT_READ | PROT_WRITE, MAP_SHARED,shm->fd,0);

    if (shm == MAP_FAILED)
    {
         printf("FAILED TO MAP shared memory segment.\n");
    }
    printf("Created shared memory segment.\n");
    printf("ADDRESS OF PARKING %p\n",shm);
    shm->test = (char*)'c';

    return shm;
    
}



int main()
{   
   
    parking_data_t parking; //Initilize parking segment
    parking_data_t *shm;    //Holds Mapped Shm Address

    
    //Map Parking Segment to Memory and retrive address.
    shm = create_shared_memory(&parking);   
   
    //sleep(5);

    //Initilized Pointers to Memory Address of Shared Segment
     createPointers(shm);    
     
    
     //Check Pointers are mapped correctly     
    printf("SHM VALUE %p\n",(char*)(shm + 184));   //Should match Entrace Boomgate Value below
    printf("SHM VALUE %p\n",shm->entrys[0].boomgate); //Print Address
    printf("SHM VALUE %c\n",shm->entrys[0].boomgate); //Print Value

   
    
    if((munmap (shm, SHMSZ )) ==-1){
		perror("munmap failed");
	} 
	   
    
    
    return 0;




}