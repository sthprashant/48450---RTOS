/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Name: Prashant Shrestha
Student Number: 98136563
Task:Program 1 (15pts)
**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_1.c -o prog_1 -lpthread -lrt

*/

#include <pthread.h> /* pthread functions and data structures for pipe */
#include <unistd.h>  /* for POSIX API */
#include <stdlib.h>  /* for exit() function */
#include <stdio.h>   /* standard I/O routines */
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

//Maximum processes
#define PROCESSLEN 7

//FIFO directory path
#define FIFODIR "/tmp/fifostore"

typedef struct SRTF_Params
{
    int pid; // process id

    // process time
    int tArrive, tWait, tBurst, tTurnaround, tRemain;
} Process_Params;

typedef struct
{
    sem_t *writeSem;
    sem_t *readSem;
} Worker1_t;

typedef struct
{
    sem_t *readSem;
    sem_t *writeSem;
    FILE *fp;
} Worker2_t;

Process_Params processes[7];

//index intialisation

char string[256];

int checkStat = 0;

float averageWaitTime, averageTurnAroundTime = 0.0;

//semaphore
sem_t writeSem; // to write to FIFO
sem_t readSem;  // to read from FIFO

//Pthreads
pthread_t thread1, thread2;

//Prototypes
void processCreate();
void Semaphore_Init();
void FIFO_Create();
void FIFO_Exec();
void FIFO_Delete();
int writeDatatoFile(FILE *des, char *buff);
void SRTF_Process(/*Process_Params *processes*/);
void calcAvg();
void *worker1(Worker1_t *data);
void *worker2(Worker2_t *data);
void Print_Result();

//Function creates process id
void processCreate(/*Process_Params *processes*/)
{
    int i;

    //Process ID			//Arrive Time				//Burst Time
    processes[0].pid = 1;   processes[0].tArrive = 8;   processes[0].tBurst = 10;
    processes[1].pid = 2;   processes[1].tArrive = 10;  processes[1].tBurst = 3;
    processes[2].pid = 3;   processes[2].tArrive = 14;  processes[2].tBurst = 7;
    processes[3].pid = 4;   processes[3].tArrive = 9;   processes[3].tBurst = 5;
    processes[4].pid = 5;   processes[4].tArrive = 16;  processes[4].tBurst = 4;
    processes[5].pid = 6;   processes[5].tArrive = 21;  processes[5].tBurst = 6;
    processes[6].pid = 7;   processes[6].tArrive = 26;  processes[6].tBurst = 2;

        for (i = 0; i < PROCESSLEN; i++)
    {
        processes[i].tRemain = processes[i].tBurst;
    }
}

//function intialises semaphores
void Semaphore_Init()
{
    if (sem_init(&writeSem, 0, 1))
    {
        perror("ERROR: Unable to initialise Semaphore write");
        exit(1);
    }

    if (sem_init(&readSem, 0, 0))
    {
        perror("ERROR: Unable to initialise Semaphore read");
        exit(1);
    }
}

//creating and initialising FIFO
void FIFO_Create()
{
    int writefd;
    int check;

    //creating FIFO (named pipe)
    check = mkfifo(FIFODIR, 0666);

    //checking if FIFO can be created
    if (check != 0)
    {
        perror("ERROR: Unable to create FIFO");
        exit(EXIT_FAILURE);
    }

    //checking if FIFO can be opened
    if (writefd = open(FIFODIR, O_RDONLY | O_NONBLOCK) < 0)
    {
        perror("ERROR: Unable to open file");
        FIFO_Delete();
        exit(EXIT_FAILURE);
    }

    else
    {
        printf("MSG: FIFO received \n");
    }

    sleep(1);

    close(writefd);
}

//writes to the FIFO
void FIFO_Exec()
{
    int writefd;

    if ((writefd = open(FIFODIR, O_WRONLY)) < 0)
    {
        printf("ERROR: Unable to open File\n");
        exit(EXIT_FAILURE);
    }

    write(writefd, string, strlen(string));

    close(writefd);
}

//writes data to txt file
int writeDatatoFile(FILE *des, char *buff)
{
    //check if data can written to file
    if (checkStat != 0)
    {
        fputs(buff, des);
        printf("STATUS: FIFO data wrting to file\n%s", buff);
    }

    else
    {
        printf("ERROR: Unable to write data from FIFO to File.\n");
        printf("MSG: Program will now close.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

//deletes and unlink FIFO
void FIFO_Delete()
{
    if (unlink(FIFODIR) < 0)
    {
        perror("ERROR: Unable to remove FIFO \n");
        exit(EXIT_FAILURE);
    }

    else
    {
        printf("MSG: Program Successful\n\n");
        exit(EXIT_SUCCESS);
    }
}

//SRTF scheduling is done using this function
void SRTF_Process()
{
    int i, smallest, time, endTime = 0;
    int remain = 0;

    //Placeholder for remaining time
    processes[7].tRemain = 9999;

    for (time = 0; remain != PROCESSLEN; time++)
    {
        //Placeholder for smallest
        smallest = 7;

        for (i = 0; i < PROCESSLEN; i++)
        {
            if (processes[i].tArrive <= time &&
                processes[i].tRemain < processes[smallest].tRemain &&
                processes[i].tRemain > 0)
            {
                smallest = i;
            }
        }

        //time increases, remaining time decreses
        processes[smallest].tRemain--;

        /*
		save the time info and increase remain
		after the process is complete
		*/
        if (processes[smallest].tRemain == 0)
        {
            remain++;
            endTime = time + 1;

            processes[smallest].tTurnaround = endTime - processes[smallest].tArrive;
            processes[smallest].tWait = endTime - processes[smallest].tBurst - processes[smallest].tArrive;

            averageWaitTime += processes[smallest].tWait;
            averageTurnAroundTime += processes[smallest].tTurnaround;
            checkStat++;
        }
    }
}

//formats the output and saves it to string
void stringStore()
{
    sprintf(string,
            "Average wait time: %f \nAverage turnaround time: %f \n",
            averageWaitTime, averageTurnAroundTime);
}

//calculate average wait and turnaround time
void calcAvg()
{
    averageWaitTime /= PROCESSLEN;
    averageTurnAroundTime /= PROCESSLEN;
}

/* this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(Worker1_t *data)
{
    //checking semaphore write availability
    sem_wait(data->writeSem);
    processCreate();
    printf("Sucessfully initialised process\n");
    SRTF_Process();
    printf("scheduling successful\n");

    calcAvg();

    FIFO_Create();

    stringStore();

    FIFO_Exec();

    sem_post(data->readSem);

    pthread_exit(NULL);
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2(Worker2_t *data)
{

    //check semaphore read
    sem_wait(data->readSem);

    int len, writefd;
    int eof = 0;
    char string[256];

    //opening FIFO using Read mode
    if (writefd = open(FIFODIR, O_RDONLY | O_NONBLOCK) < 0)
    {
        perror("ERROR: Unable to open FIFO\n");
        exit(EXIT_FAILURE);
    }

    // reading the FIFO data
    len = read(writefd, string, 256);

    //check if FIFO is empty
    if (len == 0)
        printf("ERROR:FIFO is empty\n");

    //write until the data has reached end of file
    else if (!eof)
    {
        // printf("testing%s\n", string);
        writeDatatoFile(data->fp, string);
    }

    //if end of file, exit
    else if (eof)
        exit(EXIT_SUCCESS);

    //printing the output in console
    Print_Result();

    //clear, close and remove FIFO
    close(writefd);
    FIFO_Delete();
    sem_post(data->writeSem);

    pthread_exit(NULL);
}

//Printing the results
void Print_Result()
{
    int i;
    printf("\nProcess Schedule Table: \n");

    printf("\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n");

    for (i = 0; i < PROCESSLEN; i++)
    {
        printf("\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].pid, processes[i].tArrive, processes[i].tBurst, processes[i].tWait, processes[i].tTurnaround);
    }

    printf("\nAverage wait time: %fs\n", averageWaitTime);

    printf("\nAverage turnaround time: %fs\n\n", averageTurnAroundTime);
}

/* this main function creates named pipe and threads */
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("ERROR: 2 Arguments required");
        return -1;
    }
    printf("Starting Program... \n");

    //output text file being opened
    FILE *stream = fopen(argv[1], "w");

    //removing any exixting FIFO
    unlink(FIFODIR);

    if (!stream)
    {
        printf("ERROR: Unable to open file.");
        exit(EXIT_FAILURE);
    }

    //Initialising semaphore
    Semaphore_Init();
    printf("MSG: Successfully initialised Semaphore\n");

    Worker1_t t1 = {&writeSem, &readSem};
    Worker2_t t2 = {&readSem, &writeSem, stream};

    //creating threads

    if (pthread_create(&thread1, NULL, (void *)worker1, &t1) != 0)
    {
        printf("ERROR: Error creating thread 1 \n");
        return -1;
    }

    printf("MSG: Thread 1 has been created.\n");

    printf("MSG: Creating Thread 2.\n");

    if (pthread_create(&thread2, NULL, (void *)worker2, &t2) != 0)
    {
        printf("ERROR: Error creating thread 2 \n");
        return -2;
    }

    printf("Thread 2 created successfully!\n");

    if (pthread_join(thread1, NULL) != 0)
    {
        printf("ERROR: join thread 1 error\n");
        return -3;
    }
    if (pthread_join(thread2, NULL) != 0)
    {
        printf("ERROR: join thread 2 error\n");
        return -4;
    }

    if (sem_destroy(&writeSem) != 0 &&
        sem_destroy(&readSem) != 0)
    {
        printf("ERROR: Unable to terminate Semaphore");
        return -5;
    }

    if (!stream)
    {
        if (fclose(stream))
        {
            printf("ERROR: Unable To close file. \n");
            exit(EXIT_FAILURE);
        }
    }

    //deleting FIFO
    FIFO_Delete();

    exit(EXIT_SUCCESS);
}
