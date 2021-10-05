/*
* IOS - proj2
*
* @author xtichy31
* @date 2.5.2021
* @brief The Santa Claus probleem
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>

// Macros to easily allocate and deallocate shared memory

#define ALOC_SHARED(pointer)                                                                                   \
    {                                                                                                          \
        (pointer) = mmap(NULL, sizeof(*(pointer)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); \
    }

#define DEL_SHARED(pointer)                    \
    {                                          \
        munmap((pointer), sizeof(*(pointer))); \
    }

struct sharedVariables_t
{
    int *A;
    int *rdID;
    int *waitingElfes;
    int *elfID;
    int *takenHolidays;
    bool *christmas;
    int *rd_counter;
    bool *endingElfes;
} sharedVariables;

struct sharedSemaphores_t
{
    sem_t *semaphore_gate;
    sem_t *semaphore_elfs;
    sem_t *workshop;
    sem_t *semaphore_santa;
    sem_t *semaphore_santa_help;
    sem_t *semaphore_reindeers;
    sem_t *message_semaphore;
    sem_t *semaphore_ending;
} sharedSemaphores;

struct arguments_t
{
    int numOfElfes;
    int numOfReindeers;
    int elfWorkingTime;
    int rdReturningTime;
} arguments;

// Function to allocate and create semaphores

void allocateSharedSemaphores()
{
    sharedSemaphores.message_semaphore = NULL;
    ALOC_SHARED(sharedSemaphores.message_semaphore);
    sharedSemaphores.message_semaphore = sem_open("/xtichy31.ios.proj2.message_semaphore", O_CREAT | O_EXCL, 0666, 1);

    sharedSemaphores.semaphore_elfs = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_elfs);
    sharedSemaphores.semaphore_elfs = sem_open("/xtichy31.ios.proj2.elfs", O_CREAT | O_EXCL, 0666, 0);

    sharedSemaphores.workshop = NULL;
    ALOC_SHARED(sharedSemaphores.workshop);
    sharedSemaphores.workshop = sem_open("/xtichy31.ios.proj2.workshop", O_CREAT | O_EXCL, 0666, 3);

    sharedSemaphores.semaphore_santa = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_santa);
    sharedSemaphores.semaphore_santa = sem_open("/xtichy31.ios.proj2.santa", O_CREAT | O_EXCL, 0666, 0);

    sharedSemaphores.semaphore_santa_help = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_santa_help);
    sharedSemaphores.semaphore_santa_help = sem_open("/xtichy31.ios.proj2.santa_help", O_CREAT | O_EXCL, 0666, 0);

    sharedSemaphores.semaphore_reindeers = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_reindeers);
    sharedSemaphores.semaphore_reindeers = sem_open("/xtichy31.ios.proj2.reindeers", O_CREAT | O_EXCL, 0666, 0);

    sharedSemaphores.semaphore_gate = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_gate);
    sharedSemaphores.semaphore_gate = sem_open("/xtichy31.ios.proj2.gate", O_CREAT | O_EXCL, 0666, 1);

    sharedSemaphores.semaphore_ending = NULL;
    ALOC_SHARED(sharedSemaphores.semaphore_ending);
    sharedSemaphores.semaphore_ending = sem_open("/xtichy31.ios.proj2.ending", O_CREAT | O_EXCL, 0666, 0);
}

// Function to allocate shared memory

void allocateSharedMemory()
{
    sharedVariables.A = NULL;
    ALOC_SHARED(sharedVariables.A);
    *sharedVariables.A = 1;

    sharedVariables.waitingElfes = NULL;
    ALOC_SHARED(sharedVariables.waitingElfes);
    *sharedVariables.waitingElfes = 0;

    sharedVariables.rdID = NULL;
    ALOC_SHARED(sharedVariables.rdID);
    *sharedVariables.rdID = 0;

    sharedVariables.elfID = NULL;
    ALOC_SHARED(sharedVariables.elfID);
    *sharedVariables.elfID = 0;

    sharedVariables.christmas = NULL;
    ALOC_SHARED(sharedVariables.christmas);
    *sharedVariables.christmas = false;

    sharedVariables.rd_counter = NULL;
    ALOC_SHARED(sharedVariables.rd_counter);
    *sharedVariables.rd_counter = arguments.numOfReindeers;

    sharedVariables.endingElfes = NULL;
    ALOC_SHARED(sharedVariables.endingElfes);
    *sharedVariables.endingElfes = false;
}

// Function to destroy shared memory

void destroySharedMemory()
{
    DEL_SHARED(sharedVariables.waitingElfes);
    DEL_SHARED(sharedVariables.A);
    DEL_SHARED(sharedVariables.rdID);
    DEL_SHARED(sharedVariables.elfID);
    DEL_SHARED(sharedVariables.christmas);
    DEL_SHARED(sharedVariables.rd_counter);
    DEL_SHARED(sharedVariables.endingElfes);
    DEL_SHARED(sharedSemaphores.semaphore_santa);
    DEL_SHARED(sharedSemaphores.semaphore_elfs);
    DEL_SHARED(sharedSemaphores.workshop);
    DEL_SHARED(sharedSemaphores.semaphore_reindeers);
    DEL_SHARED(sharedSemaphores.message_semaphore);
    DEL_SHARED(sharedSemaphores.semaphore_gate);
    DEL_SHARED(sharedSemaphores.semaphore_ending);
    DEL_SHARED(sharedSemaphores.semaphore_santa_help);
}

// Function to destroy semaphores

void destroySemaphores()
{
    sem_close(sharedSemaphores.semaphore_elfs);
    sem_close(sharedSemaphores.semaphore_santa);
    sem_close(sharedSemaphores.semaphore_santa_help);
    sem_close(sharedSemaphores.semaphore_reindeers);
    sem_close(sharedSemaphores.message_semaphore);
    sem_close(sharedSemaphores.semaphore_gate);
    sem_close(sharedSemaphores.semaphore_ending);
    sem_close(sharedSemaphores.workshop);
    sem_unlink("xtichy31.ios.proj2.gate");
    sem_unlink("xtichy31.ios.proj2.santa");
    sem_unlink("xtichy31.ios.proj2.santa_help");
    sem_unlink("xtichy31.ios.proj2.elfs");
    sem_unlink("xtichy31.ios.proj2.workshop");
    sem_unlink("xtichy31.ios.proj2.reindeers");
    sem_unlink("xtichy31.ios.proj2.message_semaphore");
    sem_unlink("xtichy31.ios.proj2.ending");
}
// function to check if the valid number of arguments was entered

void validNumOfArguments(int numOfArguments)
{
    if (numOfArguments < 5 || numOfArguments > 5)
    {
        fprintf(stderr, "Enter the right number of arguments, please!\n");
        exit(1);
    }
}

// function to check if arguments are only integers

void validArguments(int numOfArguments, char *arguments[])
{
    for (int i = 1; i < numOfArguments; i++)
    {
        int k = atoi(arguments[i]);
        if (k == 0 && strlen(arguments[i]) > 1)
        {
            fprintf(stderr, "Please, enter integers only!\n");
            exit(1);
        }
    }
}

// function to check if arguments are in the right range

void rangeOfArguments(char *arguments[])
{
    if (!(atoi(arguments[1]) > 0 && atoi(arguments[1]) < 1000))
    {
        fprintf(stderr, "The number of elves has to be greater than 0 and lower than 1000!\n");
        exit(1);
    }
    if (!(atoi(arguments[2]) > 0 && atoi(arguments[2]) < 20))
    {
        fprintf(stderr, "The number of reindeers has to be greater than 0 and lower than 20!\n");
        exit(1);
    }
    if (!(atoi(arguments[3]) >= 0 && atoi(arguments[3]) < 1000))
    {
        fprintf(stderr, "The time when elf is working has to be greater than 0 and lower than 1000!\n");
        exit(1);
    }
    if (!(atoi(arguments[4]) >= 0 && atoi(arguments[4]) < 1000))
    {
        fprintf(stderr, "The reindeer-coming-home time has to be greater than 0 and lower than 1000!\n");
        exit(1);
    }
}
int randomNumber(int maximum, int minimum)
{
    return (rand() % (maximum + 1 - minimum) + minimum);
}

// process Santa

void processSanta(FILE *file, struct sharedVariables_t sharedV, struct sharedSemaphores_t sharedS)
{

    // Starting the Santa process

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: Santa: going to sleep\n", *sharedV.A);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    while (true)
    {

        // Waiting for elves, so he can help them

        sem_wait(sharedS.semaphore_santa);
        sem_wait(sharedS.semaphore_gate);

        // If all reindeers are waiting to get hitched, Santa closes worshop

        if ((*sharedV.christmas))
        {
            break;
        }

        sem_wait(sharedS.message_semaphore);
        fprintf(file, "%d: Santa: helping elves\n", *sharedV.A);
        *sharedV.A += 1;
        fflush(file);
        sem_post(sharedS.message_semaphore);

        // Santa helped elfs

        sem_post(sharedS.semaphore_santa_help);
        sem_post(sharedS.semaphore_gate);

        // Santa waiting for elves to 'print' get help

        sem_wait(sharedS.semaphore_elfs);

        // Elves have been served, so Santa can go sleep

        sem_wait(sharedS.message_semaphore);
        fprintf(file, "%d: Santa: going to sleep\n", *sharedV.A);
        *sharedV.A += 1;
        fflush(file);
        sem_post(sharedS.message_semaphore);
    }

    // Santa is closing workshop

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: Santa: closing workshop\n", *sharedV.A);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    // All the reindeers are going to get hitched

    sem_post(sharedS.semaphore_reindeers);

    // Waiting for all the reindeers to get hitched

    sem_wait(sharedS.semaphore_ending);

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: Santa: Christmas started\n", *sharedV.A);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    // Christmas has started so we can end elfves' loop

    (*sharedV.endingElfes) = true;
    sem_post(sharedS.semaphore_gate);
    sem_post(sharedS.semaphore_santa_help);

    fclose(file);
    exit(0);
}

// process Elf

void processElf(FILE *file, struct sharedVariables_t sharedV, struct sharedSemaphores_t sharedS, int elfID, struct arguments_t args)
{

    // Starting Elf process

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: Elf %d: started\n", *sharedV.A, elfID);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    while (true)
    {

        // Elf is working

        if (args.elfWorkingTime != 0)
        {
            int randomNumber = (rand() % (args.elfWorkingTime));
            usleep(randomNumber * 1000);
        }

        // Elf needs help

        sem_wait(sharedS.message_semaphore);
        fprintf(file, "%d: Elf %d: need help\n", *sharedV.A, elfID);
        *sharedV.A += 1;
        fflush(file);
        sem_post(sharedS.message_semaphore);

        // Waiting for empty workshop

        sem_wait(sharedS.workshop);
        sem_wait(sharedS.semaphore_gate);

        // Checking if elves should end their work and take holidays

        if (*sharedV.endingElfes)
        {
            sem_post(sharedS.semaphore_gate);
            break;
        }
        *sharedV.waitingElfes += 1;

        // Checking if there 3 elves waiting to get help

        if (*sharedV.waitingElfes == 3)
        {
            // If so, Santa is going to help them
            sem_post(sharedS.semaphore_santa);
        }

        // Waiting for santa to help elves

        sem_post(sharedS.semaphore_gate);
        sem_wait(sharedS.semaphore_santa_help);

        // Checking if elves should end their work and take holidays

        if (*sharedV.endingElfes)
        {
            break;
        }
        *sharedV.waitingElfes -= 1;

        sem_wait(sharedS.message_semaphore);
        fprintf(file, "%d: Elf %d: get help\n", *sharedV.A, elfID);
        *sharedV.A += 1;
        fflush(file);
        sem_post(sharedS.message_semaphore);

        // 3 elves got help and other elves can demand help

        if (*sharedV.waitingElfes == 0)
        {
            sem_post(sharedS.semaphore_elfs);
            sem_post(sharedS.workshop);
            sem_post(sharedS.workshop);
            sem_post(sharedS.workshop);
        }
        else
        {
            sem_post(sharedS.semaphore_santa_help);
        }
    }

    // Waiting for all reindeers to get hitched and Santa announcing the start of Christmas

    sem_wait(sharedS.semaphore_reindeers);

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: Elf %d: taking holidays\n", *sharedV.A, elfID);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    sem_post(sharedS.semaphore_reindeers);
    sem_post(sharedS.workshop);
    sem_post(sharedS.semaphore_santa_help);

    fclose(file);
    exit(0);
}

// process Reindeer

void processReindeer(FILE *file, struct sharedVariables_t sharedV, struct sharedSemaphores_t sharedS, int rdID, struct arguments_t args)
{

    // Starting Reindeer process

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: RD %d: rstarted\n", *sharedV.A, rdID);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    // Simulating the time spent on holiday

    if (args.rdReturningTime != 0)
    {
        int randomNumber = (rand() % ((args.rdReturningTime / 2) + (args.rdReturningTime / 2)));
        usleep(randomNumber * 1000);
    }

    // Returned from the holiday

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: RD %d: return home\n", *sharedV.A, rdID);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    // If all the reindeers returned home, Santa is gonna close workshop

    sem_wait(sharedS.semaphore_gate);
    *sharedV.rd_counter -= 1;
    if (*sharedV.rd_counter == 0)
    {
        *sharedV.christmas = true;
        sem_post(sharedS.semaphore_santa);
    }
    sem_post(sharedS.semaphore_gate);
    sem_wait(sharedS.semaphore_reindeers);

    // All the reindeers are gonna get hitched

    sem_wait(sharedS.message_semaphore);
    fprintf(file, "%d: RD %d: get hitched\n", *sharedV.A, rdID);
    *sharedV.A += 1;
    fflush(file);
    sem_post(sharedS.message_semaphore);

    // When all the reindeers are hitched the Christmas will start

    *sharedV.rd_counter += 1;
    if (*sharedV.rd_counter == args.numOfReindeers)
    {
        sem_post(sharedS.semaphore_ending);
    }

    // Now all the elves can take holidays

    sem_post(sharedS.semaphore_reindeers);

    fclose(file);
    exit(0);
}

///      ///
/// MAIN ///
///      ///

int main(int argv, char *argc[])
{
    validNumOfArguments(argv);
    validArguments(argv, argc);
    rangeOfArguments(argc);
    arguments.numOfElfes = atoi(argc[1]);
    arguments.numOfReindeers = atoi(argc[2]);
    arguments.elfWorkingTime = atoi(argc[3]);
    arguments.rdReturningTime = atoi(argc[4]);
    allocateSharedMemory();
    allocateSharedSemaphores();

    FILE *file = fopen("proj2.out", "w");
    if (file == NULL)
    {
        fprintf(stderr, "The file cannot be opened!\n");
        destroySharedMemory();
        destroySemaphores();
        return -1;
    }

    // Creating process Santa, NE elf processes, NR reindeer processes

    int santaPID, elfPID, reindeerPID;

    // Santa process

    santaPID = fork();
    if (santaPID == 0)
    {
        processSanta(file, sharedVariables, sharedSemaphores);
    }
    else if (santaPID > 0)
    {
        for (int i = 0; i < arguments.numOfElfes; i++)
        {
            elfPID = fork();
            if (elfPID == 0)
            {

                // Elf process

                processElf(file, sharedVariables, sharedSemaphores, i + 1, arguments);
            }
            else if (elfPID < 0)
            {
                fprintf(stderr, "The elf process wasn't succesfully created, ending program...\n");
                destroySharedMemory();
                destroySemaphores();
                exit(1);
            }
        }
        for (int i = 0; i < arguments.numOfReindeers; i++)
        {
            reindeerPID = fork();
            if (reindeerPID == 0)
            {

                // Reindeer process

                processReindeer(file, sharedVariables, sharedSemaphores, i + 1, arguments);
            }
            else if (reindeerPID < 0)
            {
                fprintf(stderr, "The reinderr process wasn't succesfully created, ending program...\n");
                destroySharedMemory();
                destroySemaphores();
                exit(1);
            }
        }
    }
    else
    {
        fprintf(stderr, "The santa process wasn't succesfully created, ending program...\n");
        destroySharedMemory();
        destroySemaphores();
        exit(1);
    }

    // Destroying shared memory and semaphores

    fclose(file);
    destroySharedMemory();
    destroySemaphores();
    return 0;

    // THE END OF FILE proj2.c
}