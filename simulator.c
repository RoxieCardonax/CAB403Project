// Needs to handle queue's for entrances and exits
// Needs to handle the time that the car has been in there
// Simulat the time it takes for the boom gates to opperate
#include "resources/generatePlate.h"
#include "resources/hashTable.h"
int main(int argc, char **argv)
{
    // create a hash table with 100 buckets
    htable_t h;
    if (!htable_init(&h, 5))
    {
        printf("failed to initialise hash table\n");
        return EXIT_FAILURE;
    }

    // add 100 items to hash table.
    for (int i = 0; i < 15; i++)
    {
        // Generate numberplate and add to hashtable
        htable_add(&h, generateNumberPlate());
    }

    htable_print(&h);
}