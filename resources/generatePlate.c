#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#define SHMSZ 2920
#define Num_Of_Entries 5
#define Num_Of_Exits 5
#define Num_Of_Level 5
#define Max_Per_Level 20

// Generate random number
int randomNumber()
{
    // Get random number
    int random = rand();

    return random;
}

// Import plates from file
FILE *importPlates()
{

    // Get file pointer
    FILE *plates;

    // Open file
    plates = fopen("./resources/plates.txt", "r");

    return plates;
}

// Number plate generator - 50/50 from list or random
char *generateNumberPlate(FILE *plates)
{

    // Random numbe for testing
    int rand = randomNumber();

    // 50/50 whether car from list
    if (rand % 2 == 0)
    {
        // Pick from list
        // Get random number under 100
        rand = randomNumber() % 100;

        // Counter for choosing random plate from  file
        int counter = 0;

        // String in progress
        char *numberPlate = malloc(sizeof(char) * 7);

        // Loop through all characters in plates
        for (char c = getc(plates); c != EOF; c = getc(plates))
        {
            // If not new line char, add to string
            if (c != '\n')
            {
                // Add char to string
                strncat(numberPlate, &c, 1);
            }
            else
            {
                // Increment num lines
                counter++;

                // Return a number plate after a random number of lines under
                if (counter == rand)
                {

                    // Return current - after it is used it needs to be freed
                    return numberPlate;
                }

                // If not returning numberPlate, reset it
                numberPlate[0] = '\0';
            }
        }
    }
    else
    {
        // Randommly generated number plate
        // ASCII limits for char
        int alphabetStart = 65;
        int alphabetEnd = 90;

        // ASCII limits for numbers
        int numStart = 48;
        int numEnd = 57;

        // Init number plate
        char *numberPlate = malloc(6 * sizeof(int));

        // Numbers for the first 3
        for (int i = 0; i < 3; i++)
        {
            numberPlate[i] = (randomNumber() % (numEnd - numStart)) + numStart;
        }

        // Alphatbet chars for the last 3
        for (int i = 3; i < 6; i++)
        {
            numberPlate[i] = (randomNumber() % (alphabetEnd - alphabetStart)) + alphabetStart;
        }

        // Memory needs to be freed when car leaves
        return (numberPlate);
    }
};

int main()
{
    // Init for random numbers
    time_t t1;
    srand((unsigned)time(&t1)); // pass the srand() parameter

    FILE *plates = importPlates();

    // Generate license plates
    printf("%s\n", generateNumberPlate(plates));
}