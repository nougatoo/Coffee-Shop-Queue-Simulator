/*
 * Brandon Brien
 * 10079883
 * 
 * 
 * If both baristas are free and they both take a customer
 * then either of them could finish first
 * 
 * Theres a bug that sometimes when i run the simulation
 * the average time is just huge, for a quick fix 
 * just make a clause that if the time was larager than 2bil 
 * we dont add it, it's obviously and error. Might be with my code
 * or it might be with time of day, i'm not really sure. RESULSTs in 
 * ~ 0.7% data loss 
 *
 * The time for orders gradually goes down as the simulation
 * is run longer and longer. Not sure why.
 */


#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/queue.h>
#include <time.h>

pthread_t tid[10000];
int counter;
pthread_mutex_t lock;
int queue[30];

int front;
int b_one_inUse; /* 1 means in use 0 means NOT in use */
int b_two_inUse; /* 1 means in use 0 means NOT in use. */
int usage;

double average;
double simpleTime;
double complexTime;
double numSimple;
double numComplex;
double totalTime;
double totalWaitTime;

FILE *fp;

struct person {
	int order;
	
};

void* baristas(void *z)
{	
	struct timeval start, end;
	gettimeofday(&start, NULL);

	//int jType = z.order;
	struct person *personTest = z;
	int jType = personTest->order;	 
	
	int finished = 0;
	int i;	
	int j;
	unsigned long long t;

	pthread_mutex_lock(&lock);
	int tempFront = front;
	pthread_mutex_unlock(&lock);

	/* loops through the two barista's until one ready. W
	 * When it's done with the barista then it sets the 
	 * finished flag to exit out of the loop and stuff
	 */
	while (finished == 0)
	{

		/* This if condition is acting as barista 1 */
		if (b_one_inUse == 0)
		{
	

			/* "locks barista1" so that no one else can use them */
			pthread_mutex_lock(&lock);
			b_one_inUse = 1; 

			/* Gets the first person in line  */
			jType = queue[tempFront];	
			front += 1;
			pthread_mutex_unlock(&lock);
			
			/* We don't want to lock this part */
			if (personTest->order == 1)
				for(i = 0;i<2100000;i++);

			else if (personTest->order == 0)
				for(i = 0;i<1100000;i++);
			
			pthread_mutex_lock(&lock);



			gettimeofday(&end, NULL);
			t = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec));
			
			if(personTest->order ==1 && (t < 2000000000))
			{
				complexTime  += totalWaitTime + t;
				totalWaitTime += t;
				

				numComplex++;
			}
			else if(personTest->order == 0 && (t < 2000000000))
			{
				simpleTime += totalWaitTime +t ;
				totalWaitTime += t;
	
				numSimple++;
			}

	
			//printf("%d\n", numSimple);
			finished = 1;
		
			/* "Unlocks" barista1 */
			b_one_inUse = 0; 	
			pthread_mutex_unlock(&lock);
			usage--;


	
		}
	

		/* This else-if condition is acting as barista 2 */
		else if (b_two_inUse == 0)
		{
			/* "locks barista1" so that no one else can use them */
			pthread_mutex_lock(&lock);
			b_two_inUse = 1; 


			//Gets the first person in line 
			jType = queue[tempFront];	
			front += 1;
			pthread_mutex_unlock(&lock);
			
			/* we dont want to lock this part */
			if (personTest->order == 1 )
				for(j = 0;j<2000000;j++);
			else if (personTest->order == 0)
				for(j = 0;j<1000000;j++);

			pthread_mutex_lock(&lock);

			gettimeofday(&end, NULL);
			t = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec));

			if(personTest->order ==1 && (t < 2000000000))
			{
				complexTime  += totalWaitTime + t;
				totalWaitTime += t;
				

				numComplex++;
			}
			else if(personTest->order == 0 && (t < 2000000000))
			{
				simpleTime += totalWaitTime +t ;
				totalWaitTime += t;
	
				numSimple++;
			}

			//printf("I'm Barista Two! Job %d is done. Type: %d Time(microseconds): %llu \n", (tempFront), personTest->order, t); //Debugging		
			finished = 1;	


			/* "Unlocks" barista1 */
			b_two_inUse = 0; 	

			pthread_mutex_unlock(&lock);
			usage--;

		}

	
	}//End of while loop

}//End of barista function

void initialize()
{

	/*
	 * Initializes the total time used for each job type
	 * and the count for each
	 */	
	
	totalWaitTime = 0;
	simpleTime = 0;
	complexTime = 0;
	numSimple = 0;
	numComplex = 0;
	totalTime = 0;

	b_one_inUse = 0;	
	b_two_inUse = 0;

	front = 1;
	usage = 0;

	average = 0;

}

void runTest(int num)
{

	srand(time(NULL));
	int created = 0;
	int a = num;
	int numOrders = a; //Can be updated with a switch statement or something
	int i;
	int r;



	initialize();
	
	/* 
	 * Thought that using a struct for the person would be better
	 * becaue it would allow you to add in whatever information
	 * about the person that you want; not just their order (in the future)
	 *
	 * Job type 1 = complex
	 * Job type 0 = simple
	 */

	struct person customers[a];
	for (i = 0; i<numOrders;i++)
	{	
		r = rand()%2; //Used for debugging
		customers[i].order = r;
		//printf("%d ", r); //Used for debugging
	}

	//Initializes lock
	pthread_mutex_init(&lock, NULL) != 0;

	
	struct timeval startT, endT;
	gettimeofday(&startT, NULL);

	/*
	 * The simulation simulates all the customers arriving at the same time
	 * (Waiting outside for the store to open) because it doesn't create 10000
	 * pthread simultainiously but it calculates time waited like they were
	 */

	while (created < a)
	{
		if (usage < 2)
		{		
		pthread_create(&(tid[created]), NULL, &baristas, &customers[created]);
		usage++;
		created++;
		}

	}

	//Int = #threads
	for (i = 0;i<a;i++)
	{
	
    		pthread_join(tid[i], NULL);
	}

	gettimeofday(&endT, NULL);

    pthread_mutex_destroy(&lock);

}

void displayTest(int sizeTest)
{

	double average_s = 0;
	double average_c = 0;
	int i;

	for(i = 0; i<1;i++)
	{
		runTest(sizeTest);

		average = (complexTime/(numComplex))/1000;
		average_c += average;

		average = (simpleTime/(numSimple))/1000;
		average_s += average;
	
	}

;
	fprintf(fp,"%d %0.2f %0.2f\n",sizeTest, (average_c/5), (average_s/5));
	//printf("%d %0.2f %0.2f\n",sizeTest, (average_c/5), (average_s/5));
	
	

}
int main(void)
{
	int rc;
	size_t s1;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	s1=200000;

	pthread_attr_setstacksize(&attr, 100);

	fp = fopen("data1.dat", "w");
	if(fp == NULL) {
		printf("derp\n");
		exit(0);
	}


	//printf("size complex simple \n");
	fprintf(fp,"##size complex simple \n");

	displayTest(10);
	displayTest(50);
	displayTest(100);
	displayTest(250);
	displayTest(500);
	displayTest(1000);
	displayTest(2000);
	displayTest(10000);


    return 0;
}

























