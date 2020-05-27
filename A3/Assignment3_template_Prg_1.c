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

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>	/* standard I/O routines */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define PROCESSES_MAX 7
#define FIFO_NAME "/tmp/pipe"
#define MESSAGELENGTH 256

typedef struct SRTF_Params {
  //add your variables here
  int pid; // Process ID
  int wait_t, arrive_t, burst_t, turnaround_t, remain_t; // Process time
} Process_Params;

Process_Params processes[8]

typedef struct{
sem_t *sem_write;
sem_t *sem_read;
}Thread1_struct;

typedef struct{
sem_t *sem_write;
sem_t *sem_read;
FILE *fp;
}Thread2_struct;

// Global variables
int n = 7, status = 0;
char string[256];
float avg_wait_t = 0.0, avg_turnaround_t = 0.0;

// Types of Semaphore
sem_t sem_write; // Reads from FIFO
sem_t sem_read; // Writes to FIFO

// Threads
pthread_t thread1, thread2;

void init() 
{
	int i;
	processes[0].pid = 1; processes[0].arrive_t = 8; processes[0].burst_t = 10;
	processes[1].pid = 2; processes[1].arrive_t = 10; processes[1].burst_t = 3;
	processes[2].pid = 3; processes[2].arrive_t = 14; processes[2].burst_t = 7;
	processes[3].pid = 4; processes[3].arrive_t = 9; processes[3].burst_t = 5;
	processes[4].pid = 5; processes[4].arrive_t = 16; processes[4].burst_t = 4;
	processes[5].pid = 6; processes[5].arrive_t = 21; processes[5].burst_t = 6;
	processes[6].pid = 7; processes[6].arrive_t = 26; processes[6].burst_t = 2;
	
	//Initialise remaining time to be same as burst time
	for (i = 0; i < PROCESSES_MAX; i++) {
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

/**
 * This function creates and initialises FIFO
 */
void createFifo(void)
{
    int fwrite;
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
    if (fwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0)
    {
        perror("Error opening file!\n");
        deleteFifo();
        exit(1);
    }
    else
        //FIFO obtained
        printf("FIFO obtained, data write to thread2\n");
    sleep(1);

    close(fwrite);
}

void doFIFO(void)
{
    int fwrite;
    // Open FIFO write
    if (fwrite = open(FIFO_NAME, O_WRONLY) < 0)
    {
        printf("Error FIFO for write.\n");
        exit(1);
    }

    // Write data in FIFO
    write(fwrite, string, strlen(string));

    // Close FIFO
    close(fwrite);
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

void process_SRTF() 
{
	int endTime, smallest, time, remain, sum_waittime, sum_turnaroundtime = 0;
	int i;
	printf("+-------------------------------------+");
    printf("\nProcess\t|Turnaround Time| Waiting Time\n");
    printf("+-------------------------------------+");
    processes[7].remainingTime = 9999;
    for (time = 0; remain != n; time++)
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

		processes[smallest].turnaround_t = endTime-processes[smallest].arrive_t;
			
	    processes[smallest].wait_t = endTime-processes[smallest].burst_t-processes[smallest].arrive_t;
			
	    avg_wait_t += processes[smallest].wait_t;
			
	    avg_turnaround_t += processes[smallest].turnaround_t;
        }
    }
	
}

//Simple calculate average wait time and turnaround time function
void calculate_average() {
	avg_wait_t /= PROCESSES_MAX;
	avg_turnaround_t /= PROCESSES_MAX;
}

//Print results, taken from sample
void print_results() {
	
	printf("Process Schedule Table: \n");
	
	printf("\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n");
	
	for (i = 0; i<PROCESSES_MAX; i++) {
	  	printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].pid,processes[i].arrive_t, processes[i].burst_t, processes[i].wait_t, processes[i].turnaround_t);
	}
	
	status ++;

	printf("\nAverage wait time: %fs\n", avg_wait_t);
	
	printf("\nAverage turnaround time: %fs\n", avg_turnaround_t);
}
/* this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(Thread1_struct *params)
{
    // Check write semaphore availability
    sem_wait(params->sem_write);

    // Initialise processes
    init(processes, PROCESSES_MAX);

    printf("initialisation successful!\n");

    // Srtf scheduling
    srtfProcess(processes, PROCESSES_MAX);

    printf("scheduling success\n");

    // Initialise and create FIFO
    createFifo();

    // Send formatted output to a string
    sprintf(string,
            "Average Waiting Time = %.2f \nAverage Turnaround Time = %.2f \n",
            avg_wait_t, avg_turnaround_t);

    // Write data to FIFO
    doFIFO();

    printf("\nThread 1 completion status !1");

    // Signal read semaphore 
    sem_post(params->sem_read);
    //sem_wait(data->sem_write);

    printf("\nThread 1 completion status !2\n");

    pthread_exit(NULL);
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2(Thread2_struct *params)
{
  //Check read semaphore
    sem_wait(params->sem_read);

    int n, fwrite;
    int eof = 0;
    char string[MESSAGELENGTH];

    //open FIFO(read mode)
    if (fwrite = open(FIFO_NAME, O_RDONLY | O_NONBLOCK) < 0)
    {
        perror("Could not open FIFO to read.\n");
        exit(EXIT_FAILURE);
    }

    printf("Thread 2 completion status !1\n");

    //Read data from FIFO
    n = read(fwrite, string, MESSAGELENGTH);

    //check if it is empty
    if (n == 0)
        printf("FIFO is empty.\n");
    //check for EOF
    else if (!eof)
        writeFile(params->fp, string);
    //Exit if EOF reached
    else if (eof)
        exit(0);

    printf("Cleaning the FIFO....\n");

    //close FIFO
    close(fwrite);
    printf("Closing FIFO...\n");
    printf("thread 2 completion status !2\n");

    //Remove FIFO
    deleteFifo();

    //Signal and unlock the write semaphore
    sem_post(params->sem_write);

    pthread_exit(NULL);
}

/* this main function creates named pipe and threads */
int main(int arg, char *argc[])
{
	/* creating a named pipe(FIFO) with read/write permission */
	// add your code 

	/* initialize the parameters */
	 // add your code 
	
	/* create threads */
	 // add your code
	
	/* wait for the thread to exit */
	//add your code
	
	// if terminal arguments not equal to two
    if (arg != 2)
    {
        printf("Program expecting 2 args!\n");
        return -1;
    }

    printf("Program starting \n");
    // Program arg1 is the output text file
    FILE *stream = fopen(argc[1], "w");

    // Remove FIFO if it exists
    unlink(FIFO_NAME);

    // Check if destination file can be opened for writing
    if (!stream)
    {
        printf("Error opening destination file.\n");
        exit(1);
    }

    // Initialise semaphore
    initialiseSemaphore();
    printf("semaphore init successful\n");

    // Initialise ans set up shared data structture
    Thread1_struct a = {&sem_write, &sem_read};
    Thread2_struct b = {&sem_read, &sem_write, stream};

    // Create threads
    if (pthread_create(&thread1, NULL, (void *)worker1, &a) != 0)
    {
        printf("Error creating thread1.\n");
        exit(1);
    }
    printf("Thread 1 created successfully!\n");
    if (pthread_create(&thread2, NULL, (void *)worker2, &b) != 0)
    {
        printf("Error creating thread2.\n");
        exit(1);
    }
    printf("\nThread 2 created successfully!\n");

    // Wait for thread exit
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Terminate the semaphores
    sem_destroy(&sem_write);
    sem_destroy(&sem_read);

    // Check if the destination file was closed successfully
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
	return 0;
}
