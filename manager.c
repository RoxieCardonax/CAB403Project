#include "resources/generatePlate.h"

int main()
{
    // Init for random numbers
    time_t t1;
    srand((unsigned)time(&t1)); // pass the srand() parameter

    FILE *plates = importPlates();

    // Generate license plates
    printf("%s\n", generateNumberPlate(plates));
    printf("%s\n", generateNumberPlate(plates));
    printf("%s\n", generateNumberPlate(plates));
    printf("%s\n", generateNumberPlate(plates));
}