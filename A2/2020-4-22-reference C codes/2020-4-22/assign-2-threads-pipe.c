/*
   ----- 48450 -- week 4 lab handout 1.2 ------ 
This is a program to use pipe to transfer the message from Thread A to Thread B.

Question: How this program can control the critical secion in memory? 
*/

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
#define BUFFER_SIZE 100

/* global vairable declare */
buffer_item buffer[BUFFER_SIZE];/* the buffer */


/* int fd[2]; this File descroptor is replaced by ThreadParams strcut*/

pthread_t  tid1,tid2; //Thread ID
pthread_attr_t attr; //Set of thread attributes

/*This function continously reads fd[0] for any input data byte
If available, prints */

void *reader(void *params);// thread call read function
void *writer(void *params); // thread call write function
void initializeData();

int main()
{
   int result;
   //buffer_item inputs[100]; 
   ThreadParams params;

   printf("please input a string for this A thread ->B thread:\n");
   if(fgets(params.message, sizeof(params.message), stdin)==0) {
	perror("fgets input is incorrect ");
        exit(-1);}

   void initialiszeData(); //run initialisation 

     result = pipe (params.pipeFile);
   if (result < 0){
       perror("pipe error");
       exit(1);
   }

/*create the thread 2*/
   if(pthread_create(&tid1, &attr, writer, (void *)(&params))!=0)
  {
	  perror("Error creating threads: ");
      exit(-1);
  }

sleep (1); /* this indicates that the Sync is needed even for the pipe */

   /*create the thread 1*/	
   if(pthread_create(&tid1, &attr, reader, (void *)(&params))!=0)
  {
	  perror("Error creating threads: ");
      exit(-1);
  }


   /*wait for the thread to exit*/
   pthread_join(tid1,NULL);
   pthread_join(tid2,NULL);

}


void *reader(void *params)
{

/* declare a ThreadParams to a local struct */ 
 ThreadParams *R_thread_params = (ThreadParams *)(params); 

printf ("In reading thread\n");
   while(1){
      char    ch;
      int     result;

      result = read (R_thread_params->pipeFile[0],&ch,1);
      if (result != 1) {
        perror("read");
        exit(4);
      }
      
      if(ch !='\0')	{
      printf ("Reader: %c\n", ch);}
      else {
      printf("reading pipe has completed\n");
      exit (5);}
   }
return 0;
}

//This function continously writes Alphabet into fd[1]
//Waits if no more space is available

void *writer(void *params)
{
  /* declare a ThreadParams to a local struct */ 
   ThreadParams *W_thread_params = (ThreadParams *)(params);

   int i=0;
   int result;
   buffer_item item[100];

   printf ("In writing thread\n");

   /* copy the input string into local variables*/
   //strcpy(item, (buffer_item*)param);
    strcpy(item, W_thread_params->message);

   while(item[i]!='\0')
   {
      result=write (W_thread_params->pipeFile[1], &item[i], 1);
      if (result!=1){ perror ("write"); 
	exit (2);
	}

	printf("%c", item[i]);
	i++;
   }

  /* add the '\0' in the end of the pipe */
   result=write (W_thread_params->pipeFile[1], &item[i], 1);
      if (result!=1){ perror ("write"); 
	exit (3);}
	
   printf("\nwriting pipe has finished\n"); 
	
return 0;
}


void initialiszeData(){

 /*get the default attributes*/
  pthread_attr_init(&attr);
}

