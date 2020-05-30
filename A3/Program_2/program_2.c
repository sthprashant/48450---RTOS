/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2
Name: Prashant Shrestha
Student Number: 98136563
Task: Program 2
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

//Number of pagefaults in the program
int pageFaults = 0;

/*
* size of frames,
* passed as arguments when running the program
 */
int frameSize;

//Function declaration
void SignalHandler(int signal);
int checkMaxIndex(uint storeCount[]);
int checkFrame(uint frame[], int val);

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
		printf("Argument Error: Missing number of frame as argument \n");
		printf("Enter no. of frame as argument \n");
		exit(-1);
	}

	if (!(frameSize = atoi(argv[1])) || frameSize < 0)
	{
		printf("Frame size needs to be greater than 0");
		exit(-1);
	}

	// Register Ctrl+c(SIGINT) signal and call the signal handler for the function.
	signal(SIGINT, SignalHandler);

	int i;
	// reference number
	int REFERENCESTRINGLENGTH = 24;
	//Argument from the user on the frame size, such as 4 frames in the document
	int frameSize = atoi(argv[1]);
	//Frame where we will be storing the references. -1 is equivalent to an empty value
	uint frame[REFERENCESTRINGLENGTH];
	//Reference string from the assignment outline
	int referenceString[24] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1, 7, 5};
	//Next position to write a new value to.
	int nextWritePosition = 0;
	//Current value of the reference string.
	int currentValue;
	uint storeCount[frameSize];

	//Initialise the empty frame with -1 to simulate empty values.
	for (i = 0; i < frameSize; i++)
	{
		storeCount[i] = 0;
		frame[i] = -1;
	}

	//Loop through the reference string values.
	for (i = 0; i < REFERENCESTRINGLENGTH; i++)
	{
		currentValue = referenceString[i];

		// Frames are populated only if values has not been assigned
		if (checkFrame(frame, currentValue) == -1)
		{
			int index = i % frameSize;

			if (frame[index] == -1)
				nextWritePosition = index;
			else
			{
				nextWritePosition = checkMaxIndex(storeCount);
			}
			frame[nextWritePosition] = currentValue;
			storeCount[nextWritePosition] = 0;
			pageFaults++;
		}

		// storeCount is being increased here
		for (int j = 0; j < frameSize; j++)
		{
			if (frame[j] != -1 && j != nextWritePosition)
				storeCount[j]++;
		}

		printf("Current Frame:  ");
		// printing current state of frame
		for (int i = 0; i < frameSize - 1; i++)
			printf("%d\t|\t ", frame[i]);
		printf("%d\n", frame[frameSize - 1]);
		printf("*********************************************************************\n");
	}

	//Sit here until the ctrl+c signal is given by the user.
	while (1)
	{
		sleep(1);
	}

	return 0;
}

/**
 * @brief This function finds the maximum value in the array
 * 
 * @param storeCount life of frame
 * @return the maximum value
 * **/
int checkMaxIndex(uint storeCount[])
{
	int maxValue = 0;

	for (int i = 0; i < frameSize; i++)
		if (storeCount[i] > storeCount[maxValue])
			maxValue = i;

	return maxValue;
}

/**
 * @brief Checks if the value is included in the frame
 * 
 * @param frame array of frame
 * @param val the value to be searched
 * @return the maximum value
 **/
int checkFrame(uint frame[], int val)
{
	for (int i = 0; i < frameSize; i++)
		if (frame[i] == val)
			return i;

	return -1;
}

/**
 Performs the final print when the signal is received by the program.

 @param signal An integer values for the signal passed to the function.
 */
void SignalHandler(int signal)
{
	printf("\n---------------------------------------------------------------------\n");
	printf("\nTotal page faults: %d\n", pageFaults);
	printf("\n---------------------------------------------------------------------\n");
	exit(0);
}
