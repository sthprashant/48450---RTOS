/***********************
   read_a_text.file

**********************/

#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>


/* --- Structs --- */

typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_A_to_B, sem_B_to_A, sem_C_to_A;
  char message[255];
} ThreadParams;

typedef char buffer_item;
#define BUFFER_SIZE 255

/* global vairable declare */
buffer_item buffer[BUFFER_SIZE];/* the buffer */

int main(int argc, char** argv) {

	ThreadParams params;
	/*char c[100]; comment the c[100] and use message[255]*/

	FILE* fptr;
	char file_name[100];
	int sig;
	char check[12] = "end_header\n";

	/* Verify the correct number of arguments were passed in */
	if (argc != 2) {
		fprintf(stderr, "USAGE:./main.out data.txt\n");
	}

	strcpy(file_name, argv[1]); //copy a string from the commond line to get the file name


	if ((fptr = fopen(file_name, "r")) == NULL) {
		printf("Error! opening file");
		// Program exits if file pointer returns NULL.
		exit(1);
	}

	// reads text until newline is encountered
	printf("reading from the file:\n");

	// indicate whether the program read to the end of header
	sig = 0;

	while (fgets(params.message, sizeof(params.message), fptr) != NULL) {
		
		//if the program read to the end of header
		//then, read the data region and print it to the console (sig==1)
		if (sig == 1)
		  fputs(params.message, stdout);
				

		/* check whether this line is the end of header,
		the new line in array c contains "end_header\n"*/
		if ((sig==0) && strcmp(params.message, check)==0)
		{
		//Yes. The end of header
			sig = 1;
		}			
	}

	fclose(fptr);

	return 0;
}


