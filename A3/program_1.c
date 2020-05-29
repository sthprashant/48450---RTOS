/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

**********************************************************************************/
/*
 *Authors: Roshish Karmacharya
 *Student ID: 11823871
 *Date: 29/05/2020
 *Topic: Simulation of CPU scheduling by applying Shortest-Remaining-Time-First algorithm (SRTF)
*/

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

// Initial values
#define PROCESSES_MAX 7
#define MESSAGELENGTH 256
#define FIFO_NAME "/tmp/pipe"

typedef struct SRTF_Params
{
    //add your variables here
    int pid;                                               // Process ID
    int wait_t, arrive_t, burst_t, turnaround_t, remain_t; // Process time
} Process_Params;

// Array of processes with 1 extra for placeholder remain_t
Process_Params processes[8];

// Worker 1 parameters
typedef struct
{
    sem_t *sem_write;
    sem_t *sem_read;
} Thread1_struct;

// Worker 2 parameters
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

/* The input data of the cpu scheduling algorithm is:
--------------------------------------------------------
Process ID           Arrive time          Burst time
    1				    8                   10
    2                   10                  3
    3                   14                  7
    4                   9                   5
    5                   16                  4
    6                   21                  6
    7                   26                  2
--------------------------------------------------------
*/
/**
 Initialise and Create process arrive times and burst times, taken from assignment details
 @param processes Pointer to the array of processes 
 @return void
 */
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

/**
 Initialises read, write semaphores
 @param void  
 @return void
 */
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

/**
 Deletes FIFO
 @param void  
 @return void
 */
void deleteFifo(void)
{
    // Error removing FIFO properly
    if (unlink(FIFO_NAME) < 0)
    {
        perror("Error removing FIFO.\n");
        exit(1);
    }
    // Completion of FIFO removal
    else
    {
        printf("FIFO removed!\n");
        exit(0);
    }
}

/**
 Creates and initialise FIFO
 @param void  
 @return void
 */
void createFifo(void)
{
    int fdwrite;
    int FifoCheck;

    // Create FIFO (named pipe)
    FifoCheck = mkfifo(FIFO_NAME, 0666);

    // Checks if a FIFO can be made
    if (FifoCheck != 0)
    {
        perror("Error making FIFO!\n");
        exit(1);
    }

    // Check FIFO file handler
    if ((fdwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0))
    {
        perror("Error opening file!\n");
        deleteFifo();
        exit(1);
    }
    else
        // FIFO obtained
        printf("FIFO obtained, data write to Worker 2\n");

    sleep(1);
    close(fdwrite);// Close FIFO
}

/**
 Writes avg wait time and turnaround time to the FIFO
 @param void  
 @return void
 */
void doFIFO(void)
{
    int fdwrite;
    // Open FIFO write
    if ((fdwrite = open(FIFO_NAME, O_WRONLY) < 0))
    {
        printf("FIFO Error\n");
        exit(1);
    }

    write(fdwrite, string, strlen(string)); // Write data in FIFO
    close(fdwrite); // Close FIFO
}

/**
 Writes data from buffer to output.txt
 @param destination destination file
 @param buffer Temp data holder
 @return 0 
 */
int writeFile(FILE *destination, char *buffer)
{
    // Active status check
    if (status != 0)
    {
        fputs(buffer, destination);
        printf("\nWriting data.\n%s", buffer);
    }
    // Error obtaining data
    else
    {
        printf("\nError writing data.\n");
        exit(1);
    }

    return 0;
}

/**
 SRTF operations and calculations
 @param processes Pointer to the array of processes
 @return void
 */
void process_SRTF(Process_Params *processes)
{
    int endTime, smallest, time = 0;
    int i;
    int remain = 0;
    printf("\nProcess\t|  Arrive Time  |   Burst Time  |Turnaround Time|  Wait Time\n");
    processes[7].remain_t = 9999; // Placeholder remaining time to be replaced
    
    // Run function until remain is equal to number of processes
    for (time = 0; remain != PROCESSES_MAX; time++)
    {
        smallest = 7; // Assign placeholder remaining time as smallest

        // Check all processes that have arrived for lowest remain time then set the lowest to be smallest
        for (i = 0; i <= PROCESSES_MAX; i++)
        {
            if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0)
                smallest = i;
        }

        processes[smallest].remain_t--; // Decrease remaining time as time increases

        // If process is finished, save time information, add to average totals and increase remain
        if (processes[smallest].remain_t == 0)
        {
            remain++;
            endTime = time + 1;

            processes[smallest].turnaround_t = endTime - processes[smallest].arrive_t;

            processes[smallest].wait_t = endTime - processes[smallest].burst_t - processes[smallest].arrive_t;
           
            // Prints all values including results
            printf("\nP[%d]\t|\t%d\t|\t%d\t|\t%d\t|\t%d\t", smallest + 1, processes[smallest].arrive_t, processes[smallest].burst_t, processes[smallest].turnaround_t, processes[smallest].wait_t);

            avg_wait_t += processes[smallest].wait_t;
            avg_turnaround_t += processes[smallest].turnaround_t;

            status++;
        }
    }

    printf("\n\nAverage waiting time = %f\n", avg_wait_t / n);
    printf("Average Turnaround time = %f\n", avg_turnaround_t / n);

    // Simple calculate average wait time and turnaround time function
    avg_wait_t /= PROCESSES_MAX;
    avg_turnaround_t /= PROCESSES_MAX;
}

/**
 this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO
 @param params Pointer to the parameters of Thread1_struct
 @return void
 */
void *worker1(Thread1_struct *params)
{
    sem_wait(params->sem_write); // Check write semaphore availability

    Process_Params processes[PROCESSES_MAX];

    init(processes); // Initialise processes

    printf("initialisation successful!\n");

    process_SRTF(processes); // Srtf scheduling

    printf("scheduling success\n");

    createFifo(); // Initialise and create FIFO

    // Send output to a string
    sprintf(string,
            "Average Waiting Time = %.2f \nAverage Turnaround Time = %.2f \n",
            avg_wait_t, avg_turnaround_t);

    doFIFO(); // Write data to FIFO

    printf("\nWorker 1 completion!1");

    sem_post(params->sem_read); // Signal read semaphore

    printf("\nWorker 1 completion!2\n");
    pthread_exit(NULL);
}

/**
 reads the waiting time and turn-around time through the FIFO and writes to text file
 @param params Pointer to the parameters of Thread2_struct
 @return void
 */
void *worker2(Thread2_struct *params)
{
    sem_wait(params->sem_read); //Check read semaphore

    int n, fdwrite;
    int eof = 0;
    char string[MESSAGELENGTH];

    //open FIFO(read mode)
    if ((fdwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0))
    {
        perror("Could not open FIFO.\n");
        exit(1);
    }

    printf("Worker 2 completion!1\n");

    n = read(fdwrite, string, MESSAGELENGTH); // Read data from FIFO and save it to n

    if (n == 0) // Check if n is empty
        printf("Empty FIFO.\n");
    else if (!eof) // Check for EOF
        writeFile(params->fp, string);
    else if (eof) // Exit if EOF
        exit(0);

    printf("Cleaning FIFO.\n");

    close(fdwrite); // Close FIFO
    printf("Closing FIFO.\n");
    printf("Worker 2 completion!2\n");
    deleteFifo(); // Remove FIFO
    sem_post(params->sem_write); // Signal and unlock the write semaphore
    pthread_exit(NULL);
}

/**
 this main function creates named pipe and threads
 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program.
 @return void
 */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Program expecting 2 args!\n"); // If arguments is not 2
        return -1;
    }
    printf("Starting Program \n");
    
    FILE *stream = fopen(argv[1], "w"); // Output text file

    unlink(FIFO_NAME); // Remove FIFO 

    // Check destination file
    if (!stream)
    {
        printf("Error opening file.\n");
        exit(1);
    }

    initialiseSemaphore(); // Initialise semaphore
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
    
    if (pthread_create(&thread2, NULL, (void *)worker2, &b) != 0)
    {
        printf("Error creating Worker 2.\n");
        exit(1);
    }

    // Exit Thread
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

    deleteFifo();  // Delete FIFO
    exit(0);
}
