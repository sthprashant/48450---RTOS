/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2 template

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_1.c -o prog_1 -lpthread -lrt

*/

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <semaphore.h>

#define PROCESSES_MAX 7
#define MESSAGELENGTH 256
#define FIFO_NAME "/tmp/pipe"


typedef struct SRTF_Params
{
    //add your variables here
    int pid;                                               // Process ID
    int wait_t, arrive_t, burst_t, turnaround_t, remain_t; // Process time
} Process_Params;

Process_Params processes[8];

typedef struct
{
    sem_t *sem_write;
    sem_t *sem_read;
} Thread1_struct;

typedef struct
{
    sem_t *sem_read;
    sem_t *sem_write;
    FILE *fp;
} Thread2_struct;

// Global variables
int n = 7, status = 0;
char string[256];
float avg_wait_t = 0.0, avg_turnaround_t = 0.0;

// Types of Semaphore
sem_t sem_write; // Reads from FIFO
sem_t sem_read;  // Writes to FIFO

// Threads
pthread_t thread1, thread2;

void init(Process_Params *processes)
{
    int i;
    processes[0].pid = 1;
    processes[0].arrive_t = 8;
    processes[0].burst_t = 10;
    processes[1].pid = 2;
    processes[1].arrive_t = 10;
    processes[1].burst_t = 3;
    processes[2].pid = 3;
    processes[2].arrive_t = 14;
    processes[2].burst_t = 7;
    processes[3].pid = 4;
    processes[3].arrive_t = 9;
    processes[3].burst_t = 5;
    processes[4].pid = 5;
    processes[4].arrive_t = 16;
    processes[4].burst_t = 4;
    processes[5].pid = 6;
    processes[5].arrive_t = 21;
    processes[5].burst_t = 6;
    processes[6].pid = 7;
    processes[6].arrive_t = 26;
    processes[6].burst_t = 2;

    //Initialise remaining time to be same as burst time
    for (i = 0; i < PROCESSES_MAX; i++)
    {
        processes[i].remain_t = processes[i].burst_t;
    }
}

void initialiseSemaphore(void)
{
    if (sem_init(&sem_read, 0, 0))
    {
        perror("Initialising semaphore read error.");
        exit(1);
    }

    if (sem_init(&sem_write, 0, 1))
    {
        perror("Initialising semaphore write error.");
        exit(1);
    }
}

void deleteFifo(void)
{
    // Error in removing FIFO properly
    if (unlink(FIFO_NAME) < 0)
    {
        perror("Error removing FIFO.\n");
        exit(1);
    }

    // Completion of FIFO removal
    else
    {
        printf("Program completed successfully, FIFO removed!\n");
        exit(0);
    }
}

void createFifo(void)
{
    int fdwrite;
    int FifoCheck;

    // Create FIFO (named pipe)
    FifoCheck = mkfifo(FIFO_NAME, 0666);

    //Checks if a FIFO can be made
    if (FifoCheck != 0)
    {
        perror("Error making FIFO!\n");
        exit(1);
    }

    //Check FIFO file handler
    if ((fdwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0))
    {
        perror("Error opening file!\n");
        deleteFifo();
        exit(1);
    }
    else
        //FIFO obtained
        printf("FIFO obtained, data write to Worker 2\n");

    sleep(1);
    close(fdwrite);
}

void doFIFO(void)
{
    int fdwrite;
    // Open FIFO write
    if ((fdwrite = open(FIFO_NAME, O_WRONLY) < 0))
    {
        printf("Error FIFO for write.\n");
        exit(1);
    }

    // Write data in FIFO
    write(fdwrite, string, strlen(string));

    // Close FIFO
    close(fdwrite);
}

int writeFile(FILE *destination, char *buffer)
{
    // Active status check
    if (status != 0)
    {
        fputs(buffer, destination);
        printf("\nWriting data to file.\n%s", buffer);
    }

    // Error obtaining data
    else
    {
        printf("\nError writing data to file.\n");
        exit(1);
    }

    return 0;
}

void process_SRTF(Process_Params *processes)
{
    int endTime, smallest, time = 0;
    int i;
    int remain = 0;
    int sum_wait = 0, sum_turnaround = 0;
    printf("\nProcess\t|  Arrive Time  |   Burst Time  |Turnaround Time|  Wait Time\n");
    processes[7].remain_t = 9999;
    for (time = 0; remain != PROCESSES_MAX; time++)
    {
        smallest = 7;
        for (i = 0; i <= PROCESSES_MAX; i++)
        {
            if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0)
                smallest = i;
        }

        processes[smallest].remain_t--;

        if (processes[smallest].remain_t == 0)
        {
            remain++;
            endTime = time + 1;

            printf("\nP[%d]\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t", smallest + 1, processes[smallest].arrive_t, processes[smallest].burst_t, endTime - processes[smallest].arrive_t, endTime - processes[smallest].burst_t - processes[smallest].arrive_t);

            sum_wait += endTime - processes[smallest].burst_t - processes[smallest].arrive_t;
            sum_turnaround += endTime - processes[smallest].arrive_t;

            status++;
        }
    }

    printf("\n\nAverage waiting time = %f\n", sum_wait * 1.0 / n);
    printf("Average Turnaround time = %f\n", sum_turnaround * 1.0 / n);

    avg_wait_t = sum_wait * 1.0 / n;
    avg_turnaround_t = sum_turnaround * 1.0 / n;
}

/* this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(Thread1_struct *params)
{
    // Check write semaphore availability
    sem_wait(params->sem_write);

    Process_Params processes[PROCESSES_MAX];

    // Initialise processes
    init(processes);

    printf("initialisation successful!\n");

    // Srtf scheduling
    process_SRTF(processes);

    printf("scheduling success\n");

    // Initialise and create FIFO
    createFifo();

    // Send formatted output to a string
    sprintf(string,
            "Average Waiting Time = %.2f \nAverage Turnaround Time = %.2f \n",
            avg_wait_t, avg_turnaround_t);

    // Write data to FIFO
    doFIFO();

    printf("\nCompletion status Worker 1 !1");

    // Signal read semaphore
    sem_post(params->sem_read);
    //sem_wait(data->sem_write);

    printf("\nCompletion status Worker 1 !2\n");

    pthread_exit(NULL);
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2(Thread2_struct *params)
{
    //Check read semaphore
    sem_wait(params->sem_read);

    int n, fdwrite;
    int eof = 0;
    char string[MESSAGELENGTH];

    //open FIFO(read mode)
    if ((fdwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0))
    {
        perror("Could not open FIFO.\n");
        exit(1);
    }

    printf("Completion status Worker 2 !1\n");

    //Read data from FIFO
    n = read(fdwrite, string, MESSAGELENGTH);

    //check if it is empty
    if (n == 0)
        printf("Empty FIFO.\n");
    //check for EOF
    else if (!eof)
        writeFile(params->fp, string);
    //Exit if EOF reached
    else if (eof)
        exit(0);

    printf("Cleaning the FIFO....\n");

    //close FIFO
    close(fdwrite);
    printf("Closing FIFO...\n");
    printf("Completion status Worker 2 !2\n");

    //Remove FIFO
    deleteFifo();

    //Signal and unlock the write semaphore
    sem_post(params->sem_write);

    pthread_exit(NULL);
}

/* this main function creates named pipe and threads */
int main(int argc, char *argv[])
{
    /* creating a named pipe(FIFO) with read/write permission */
    // add your code

    /* initialize the parameters */
    // add your code

    /* create threads */
    // add your code

    /* wait for the thread to exit */
    //add your code

    // if arguments is not 2
    if (argc != 2)
    {
        printf("Program expecting 2 args!\n");
        return -1;
    }

    printf("Program starting \n");
    // output text file
    FILE *stream = fopen(argv[1], "w");

    // Remove FIFO 
    unlink(FIFO_NAME);

    // Check for destination file to write
    if (!stream)
    {
        printf("Error opening destination file.\n");
        exit(1);
    }

    // Initialise semaphore
    initialiseSemaphore();
    printf("semaphore init successful\n");

    // Initialise shared data structture
    Thread1_struct a = {&sem_write, &sem_read};
    Thread2_struct b = {&sem_read, &sem_write, stream};

    // Create threads
    if (pthread_create(&thread1, NULL, (void *)worker1, &a) != 0)
    {
        printf("Error creating Worker 1.\n");
        exit(1);
    }
    printf("Worker 1 created successfully!\n");
    if (pthread_create(&thread2, NULL, (void *)worker2, &b) != 0)
    {
        printf("Error creating Worker 2.\n");
        exit(1);
    }
    // printf("\nWorker 2 created successfully!\n");

    // Thread exit
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Terminate the semaphores
    sem_destroy(&sem_write);
    sem_destroy(&sem_read);

    // Destination file close check
    if (!stream)
    {
        if (fclose(stream))
        {
            printf("Error closing destination file.\n");
            exit(1);
        }
    }

    // Delete FIFO
    deleteFifo();

    exit(0);
}
