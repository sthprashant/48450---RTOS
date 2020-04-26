 /*
   semphore_thread.c
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

int sum;/*this variable is shared by the threads*/

/* Thread One - call this function */
void *runnerOne(void *params);
/* Thread two - call this function*/
void *runnerTwo(void *params);
/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

int main(int argc, char*argv[])
{

    pthread_t tid1,tid2;       //Thread ID
    pthread_attr_t attr; //Set of thread attributes  
    ThreadParams params;

  int flag; /* the flag to set the sem_post to lead the runnerOne or runnerTwo thread */
   	
  if(argc!=2){
	fprintf(stderr,"usage: ./P2 number  \n<note: an integer value for loop counting times & the even value will set runnerOne go first and vice versa for the odd value>\n");
	return -1;
  }
  if(atoi(argv[1])<0){
	fprintf(stderr,"%d must be >=0\n",atoi(argv[1]));
	return -1;
  }
   //copy a string to the data struct
   strcpy(params.message, argv[1]); 
	
  // Initialization
  initializeData(&params);
 
/* Get the default attributes */
   pthread_attr_init(&attr);

  printf("Initial sum=%d\n",sum);

//pthread_create(&tid1,&attr, runnerOne, (void *)(&params));

  /*create the thread 1*/
  if(pthread_create(&tid1,&attr, runnerOne, (void *)(&params))!=0)
  {
      perror("Error creating threads one: ");
      exit(-1);
  }
//pthread_create(&tid2,&attr, runnerTwo, (void *)(&params));

  /*create the thread 2*/
 if(pthread_create(&tid2,&attr, runnerTwo, (void *)(&params))!=0)
{
      perror("Error creating threads two: ");
      exit(-1);
  }

  flag=atoi(argv[1])%2; /* calcuate the module % if it is even number T1 goes first otherwise, T2 goes first! */
  switch (flag){
	case 0: sem_post(&params.sem_A_to_B); break; //unlock semaphore_one
	case 1: sem_post(&params.sem_B_to_A); break; //unlock semaphore_two
	default: printf("Invalid swtich\n" );
  }

	  	 
  /*wait for the thread to exit*/
  pthread_join(tid1,NULL);
  pthread_join(tid2,NULL);

  /* why sum values are different if we set 'one' first and vice versa ? */  
  printf("sum=%d\n",sum);

}

/*The thread will begin control in this function*/
void *runnerOne(void *params)
{
ThreadParams *One_thread_params = (ThreadParams *)(params); 

  /* waitijng to aquire the full lock */
  sem_wait(&(One_thread_params->sem_A_to_B));

  int i,upper=atoi(One_thread_params->message);
  
  printf("thread one, the first value of sum=%d\n", sum);
  for(i=0;i<=upper;i++)
    sum=sum+2*i;

  printf("thread one, the final valeu of sum=%d\n", sum);

  /* signal empty */
  sem_post(&(One_thread_params->sem_B_to_A));
 return 0;
}

/*The thread will begin control in this function*/
void *runnerTwo(void *params)
{
 ThreadParams *Two_thread_params = (ThreadParams *)(params); 

  /* waiting to aquire the full lock */
  sem_wait(&(Two_thread_params->sem_B_to_A));

  int i,upper=atoi(Two_thread_params->message);
  printf("thread two, the first value of sum=%d\n",sum);
  for(i=0;i<=upper;i++)
    sum=sum+i;
  printf("thread two, the final value of sum=%d\n",sum);

  /* signal empty */
  sem_post(&(Two_thread_params->sem_A_to_B));

 return 0;
}

void initializeData(ThreadParams *params) {
   sum=0;

   /* Initialize the named semaphore_one, 0-shared between threads, initial value is set to one=0 */
   sem_init(&(params->sem_A_to_B), 0, 0);

   /* Initialize the named semaphore_two, 0-shared between threads, initial value is set to two=0 */
   sem_init(&(params->sem_B_to_A), 0, 0);

   
}
