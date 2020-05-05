/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.
//***********************************************************************************
//***********************************************************************************
/*
 *	Authors: Roshish Karmacharya (11823871) and Prashant Shrestha (98136563)
 *	Date: 06/05/2020
 *	Topic: Pipe, Threads and Semaphore for Real Time File Reading/Writing
*/
//***********************************************************************************
/***********************************************************************************/

/*
			******Compile instruction********
 * To compile prog_1 ensure that gcc is installed and run the following command:
   gcc prog_1.c -o prog_1 -lpthread -lrt

*/


#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/time.h>

/* --- Structs --- */

typedef struct ThreadParams
{
  int pipeFile[2];
  sem_t sem_A_to_B, sem_B_to_A, sem_C_to_A;
  char message[255];
  int endOfFile; // End of file check

} ThreadParams;

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void *ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void *ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void *ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[])
{

  int result;
  pthread_t tid1, tid2, tid3; //Thread ID
  pthread_attr_t attr;

  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

  // Create pipe
  result = pipe(params.pipeFile);
  if (result < 0)
  {
    perror("pipe error");
    exit(1);
  }

  // Create Threads
  if (pthread_create(&tid1, &attr, ThreadA, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  if (pthread_create(&tid2, &attr, ThreadB, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }
  if (pthread_create(&tid3, &attr, ThreadC, (void *)(&params)) != 0)
  {
    perror("Error creating threads: ");
    exit(-1);
  }

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);

  close(params.pipeFile[0]); // Close pipe 0
  close(params.pipeFile[1]); // Close pipe 1

  pthread_cancel(tid1); // Terminate the thread execution for thread id 1
  pthread_cancel(tid2); // Terminate the thread execution for thread id 2
  pthread_cancel(tid3); // Terminate the thread execution for thread id 3
  return 0;
}

/* --- Data Initialisation --- */
void initializeData(ThreadParams *params)
{
  // Initialize Sempahores
  sem_init(&(params->sem_A_to_B), 0, 1);
  sem_init(&(params->sem_B_to_A), 0, 0);
  sem_init(&(params->sem_C_to_A), 0, 0);

  params->endOfFile = 0; // Sets end of file value to 0 and initialize it
}

/* --- Implementation of Thread A --- */
void *ThreadA(void *params)
{
  /* note: Since the data_stract is declared as pointer. the A_thread_params->message */
  ThreadParams *A_thread_params = (ThreadParams *)(params);
  static const char file[] = "data.txt"; // data.txt is stored into local variable filename

  FILE *fReader = fopen(file, "r"); // Open file to be read
  
  if (!fReader) // Validation check
  {
    perror(file);
    exit(-1);
  }

  // Wait for sem_A_to_B to finish
  while (!sem_wait(&(A_thread_params->sem_A_to_B)))
  {
    // Read the file 
    if (fgets(A_thread_params->message, sizeof(A_thread_params->message), fReader) == NULL)
    {
      A_thread_params->endOfFile = 1; // set endOfFile flag to 1 after reaching end of file
      sem_post(&(A_thread_params->sem_C_to_A)); // Signal sem_C_to_A semaphore
      break;
    }

    // Write the data from reader to the pipe
    write(A_thread_params->pipeFile[1], A_thread_params->message, strlen(A_thread_params->message) + 1);
    sem_post(&(A_thread_params->sem_C_to_A)); // Signal sem_C_to_A semaphore
  }

  close(A_thread_params->pipeFile[1]); // Close Pipe 1
  fclose(fReader); // Close file

  return NULL;
}

/* --- Implementation of Thread B --- */
void *ThreadB(void *params)
{

  ThreadParams *B_thread_params = (ThreadParams *)(params);

  // Wait for sem_C_to_A to finish
  while (!sem_wait(&(B_thread_params->sem_C_to_A)))
  {
    // Read the data from pipe 0 and push it to message
    read(B_thread_params->pipeFile[0], B_thread_params->message, sizeof(B_thread_params->message));
    sem_post(&(B_thread_params->sem_B_to_A)); // Signal sem_B_to_A semaphore

    if (B_thread_params->endOfFile == 1) // Break after reaching end of file
      break;
  }

  close(B_thread_params->pipeFile[0]); // Close Pipe 0
  return NULL;
}

/* --- Implementation of Thread C --- */
void *ThreadC(void *params)
{
  ThreadParams *C_thread_params = (ThreadParams *)(params);
  int lineCount = 0; // Initialise line counter

  static const char file[] = "output.txt"; // output.txt is stored into local variable filename

  FILE *fWriter = fopen(file, "w"); // Open file to write

  if (!fWriter) // Validation check
  {
    perror(file);
    exit(-1);
  }

// Wait for sem_B_to_A to finish
  while (!sem_wait(&(C_thread_params->sem_B_to_A)))
  {
    if (lineCount)
    {
      // Put the data in message from fWriter file
      fputs(C_thread_params->message, fWriter); 

      if (C_thread_params->endOfFile == 1) // Break after reaching end of file
        break;
    }
    else if (strstr(C_thread_params->message, "end_header")) // Check for end of message
      lineCount = 1;

    sem_post(&(C_thread_params->sem_A_to_B)); // Signal sem_A_to_B semaphore
  }

  fclose(fWriter); // Close file
  return NULL;
}
