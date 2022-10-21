#include "resources/generatePlate.h"

int main()
{
    // Init for random numbers
    time_t t1;
    srand((unsigned)time(0)); // pass the srand() parameter

    printf("%d", rand());
    FILE *plates = importPlates();

    // Generate license plates
    for (int i = 0; i < 4; i++)
    {
        char *plate = strdup(generateNumberPlate(plates));

        printf("%s\n", plate);
    }
}

// 2003654235
// 6464033605
// 884891318

// 1118196968