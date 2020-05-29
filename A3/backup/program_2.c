/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2 template

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_2.c -o prog_2 -lpthread -lrt

*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define REFERENCESTRINGLENGTH 24
//Number of pagefaults in the program
int pageFaults = 0;
int frameSize;

//Function declaration
void SignalHandler(int signal);
int IndexMax(unsigned int num[]);
int Frame(unsigned int bulk[], int check);
void PrintFrame(unsigned int frame[]);

/**
 Main routine for the program. In charge of setting up threads and the FIFO.

 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program.
 @return returns 0 upon completion.
 */
int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Please provide 1 argument: number of frames\n");
		printf("e.g. %s 4\n", argv[0]);
		exit(-1);
	}

	if (!(frameSize = atoi(argv[1])) || frameSize < 0)
	{
		printf("Please enter a number greater than 0.\n");
		exit(-1);
	}
	//Register Ctrl+c(SIGINT) signal and call the signal handler for the function.
	signal(SIGINT, SignalHandler);

	int i;
	//Frame where we will be storing the references. -1 is equivalent to an empty value
	uint frame[frameSize];
	//Reference string from the assignment outline
	int referenceString[REFERENCESTRINGLENGTH] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1, 7, 5};
	//Next position to write a new value to.
	int nextWritePosition = 0;
	//Current value of the reference string.
	int currentValue;
	//Unsigned number and its position in the frame
	uint num[frameSize];
	//Initialise the empty frame with -1 to simulate empty values.
	for (i = 0; i < frameSize; i++)
	{
		num[i] = 0;
		frame[i] = -1; // Empty frame
	}

	//Loop through the reference string values.
	for (i = 0; i < REFERENCESTRINGLENGTH; i++)
	{
		currentValue = referenceString[i];
		
		//Change the frames if the value is not empty or repeating
		if (Frame(frame, currentValue) == -1)
		{
			nextWritePosition = frame[i % frameSize] == -1 ? i % frameSize : IndexMax(num);
			frame[nextWritePosition] = currentValue;
			num[nextWritePosition] = 0;
			pageFaults++;
		}

		// Increase num of every frame excluding empty frame
		for (int j = 0; j < frameSize; j++)
			if (frame[j] != -1 && j != nextWritePosition)
				num[j]++;

		PrintFrame(frame);
	}

	//Sit here until the ctrl+c signal is given by the user.
	while (1)
	{
		sleep(1);
	}

	return 0;
}

/**
 Returns the index of the highest value in the array

 @param num the array used to store number of frames
 @return the index of the highest value in the array
 */
int IndexMax(unsigned int num[])
{
	int max = 0;

	for (int i = 0; i < frameSize; i++)
		if (num[i] > num[max])
			max = i;

	return max;
}

/**
 Checks for the value 
 @param bulk search through array
 @param check value being searched
 @return 1- index of the value, -1 - non existent 
 */
int Frame(unsigned int bulk[], int check)
{
	for (int i = 0; i < frameSize; i++)
		if (bulk[i] == check)
			return i;

	return -1;
}

/**
 Prints frames in present state
 @param frame frames to print
 */
void PrintFrame(unsigned int frame[])
{
	printf("Current state of frame: ");

	for (int i = 0; i < frameSize - 1; i++)
		printf("%d, ", frame[i]);

	printf("%d\n", frame[frameSize - 1]);
}

/**
 Performs the final print when the signal is received by the program.
 @param signal An integer values for the signal passed to the function.
 */
void SignalHandler(int signal)
{
	if (signal == SIGINT)
	{
		printf("\nTotal page faults: %d\n", pageFaults);
		exit(0);
	}
}