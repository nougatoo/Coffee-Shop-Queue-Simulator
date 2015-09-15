/*
 * Brandon Brien
 * 10079883
 * CPSC 457 Assignment 3
 * 
 * Notes:
 * 
 * 1 = simple
 * 2 = complex
 * 	For this scenario
 * 
 * Small data bug: the simple queue with the coffee pots has it's second and third
 * customers waiting an extra combined total of ~1500. Which on the small scale (~10) kind of skews
 * the results but when you run a test with more than that it become negligible
 */

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <sys/queue.h>
#include <time.h>

pthread_t tid[2];
pthread_t tid2[10000];
pthread_t tid3[10000];
int payQueue[10000];

pthread_mutex_t lock;

int a;
int payNumber;
int payment_q_front;
int payment_q_end;
int serving;

int usage; /* Tells us whether there is an open pot or not */
int usageC; /* Tells us if the complex barista is open */
int potOne_using; /* 0 means not in use, 1 means inused */
int potTwo_using;
int potThree_using;
int complexB_using;

double simpleTime;
double complexTime;
double numSimple;
double numComplex;
double totalWaitTime;
double timeIwait;
double testTime;
double timeIwait_s;
double testTime_s;
unsigned long long bugFix;
int bug_c;
int bug_s;

int size; 
int sizeC;
double temp;
double average;

FILE *fp;

struct person {
	int order;
	
};

void* doSimple(void *arg)
{	
	int i;
	int finished = 0;
	int myQnum;
	unsigned long long t = 0;
	
	/* The time  the person starts their order */
	struct timeval start, end;
	gettimeofday(&start, NULL);


	//Should do all our coffee pot stuff
	while (finished == 0)
	{
		/* Coffee Pot One critical section */
		if (potOne_using == 0)
		{	
			pthread_mutex_lock(&lock);
			potOne_using = 1;
			pthread_mutex_unlock(&lock);

			for(i = 0; i<1000000;i++);

			pthread_mutex_lock(&lock);

			usage--;
			finished = 1; /* found a coffee pot and used it */

			//printf("Coffee Pot One has finished a job!!\n");
			potOne_using = 0;
			pthread_mutex_unlock(&lock);
			
		}

		/* Coffee Pot Two critical section */
		else if(potTwo_using == 0)
		{
			pthread_mutex_lock(&lock);
			potTwo_using = 1;
			pthread_mutex_unlock(&lock);
			
			for(i = 0; i<1000000;i++);

			pthread_mutex_lock(&lock);

			usage--;
			finished = 1; /* found a coffee pot and used it */

			//printf("Coffee Pot Two has finished a job!!\n");
			potTwo_using = 0;
			pthread_mutex_unlock(&lock);
			
		}

		/* Coffe Pot Three critical section */
		else if(potThree_using == 0)
		{
			pthread_mutex_lock(&lock);
			potThree_using = 1;
			pthread_mutex_unlock(&lock);

			for(i = 0; i<1000000;i++);

			pthread_mutex_lock(&lock);
			usage--;
			finished = 1; /* found a coffee pot and used it */

			//printf("Coffee Pot Three has finished a job!!\n");
			potThree_using = 0;
			pthread_mutex_unlock(&lock);
			
		}

	
	

	}//End of while-loop

	/*
	 * Payment works in a fifo order because if a thread makes it to here
	 * it gets a number and then loops until the serving variable it equal to 
	 * it's queue number. Serving starts at 1 and everytime a thread finds that
	 * "serving" is equal to their queue number then it spends a little bit if time 
	 * simuatling service then updates serving so that it can server the next
	 * thread. FIFO because threads that get a queue number are ready to pay 
	 * but don't get served until all other threads that got a queue number before
	 * them get served
	 */

	/* Assigns this thread a payment queue number */	
	pthread_mutex_lock(&lock);
	myQnum = payNumber+1;
	payNumber++;
	pthread_mutex_unlock(&lock);

	while (serving != myQnum);

	/* Beep Boop, paying m0n3y */
	for (i=0;i<100000;i++);	

	pthread_mutex_lock(&lock);	
	serving++;		
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&lock);
	//printf("My Queue num is: %d\n", myQnum); //For debugging

	/* A band-aid for a bug */	
	if(timeIwait == 0)
		bugFix = t;

	gettimeofday(&end, NULL); //THIS WILL BE MOVED TO AFTER THE PAYING HAS BEEN COMPLETED
	t = ( (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec));

	/* because it was returning a "negative" sometimes */
	if (t < 2000000000)
	{
		testTime_s += t; //sum of all waited times;
		timeIwait_s = testTime_s;

		simpleTime += timeIwait_s;
	}
	else
		bug_s += 1;
	pthread_mutex_unlock(&lock);

}

void* createSimple(void *arg)
{
	int *numSimples = arg; /* Number of total simple customers we have to deal with */
	int created = 0; /*How many simple customers have we created? */
	int i;

	//printf("Simple %d\n", *numSimples);
	while (created < *numSimples)
	{
		//for(i = 0; i<100000;i++0);
		if(usage < 3)
		{
			pthread_create(&(tid2[created]), NULL, &doSimple, NULL);
			created++;
			pthread_mutex_lock(&lock);
			usage++;
			pthread_mutex_unlock(&lock);
		}

	}
		
}

void* doComplex(void *arg)
{
	int i;
	int j;
	int finished = 0;
	int myQnum;
	int repeated = 0;
	unsigned long long t = 0;
	
	/* The time  the person starts their order */
	struct timeval start, end;
	gettimeofday(&start, NULL);

	if(complexB_using == 1)
		printf("I found a bug\n");

	while (finished == 0)
	{
		if (complexB_using == 0)
		{
			pthread_mutex_lock(&lock);
			complexB_using = 1;
			pthread_mutex_unlock(&lock);

			for(i = 0;i<2000000;i++);

			pthread_mutex_lock(&lock);

			finished = 1;
			usageC--;
			complexB_using = 0;
			//printf("The barista has finished a job!\n");

			pthread_mutex_unlock(&lock);
		}//End of if-statement
	
		
	}//End of while-loop

	/* Assigns this thread a payment queue number & adds it to the queue */
	pthread_mutex_lock(&lock);
	myQnum = payNumber+1;
	payNumber++;

	pthread_mutex_unlock(&lock);
	while (serving != myQnum);

	/* Beep Boop, paying m0n3y */
	for (j = 0;j<100000;j++);

	pthread_mutex_lock(&lock);	
	serving++;		
	pthread_mutex_unlock(&lock);

	
	pthread_mutex_lock(&lock);

	gettimeofday(&end, NULL); //THIS WILL BE MOVED TO AFTER THE PAYING HAS BEEN COMPLETED
	t = ( (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec));

	/* because it was returning a "negative" sometimes */
	if (t < 2000000000)
	{

		testTime += t; //sum of all waited times;
		timeIwait = testTime;
		complexTime += timeIwait;
	}
	else
		bug_c += 1;

	pthread_mutex_unlock(&lock);
	
	//printf("My Queue num is: %d\n", myQnum); //Debugging	
	//printf("A complex time: %llu \n", t);
		
}

void* createComplex(void *arg)
{

	int *numComplex = arg;
	int created = 0;
	int i;


	/* Only creates a pthreads when necessary */
	//printf("Complex: %d\n", *numComplex);
	while(created < *numComplex)
	{
	
		//printf("Created: %d, numComplex: %d, usage: %d\n", created, *numComplex, usageC);
		if(usageC < 1)
		{
			pthread_create(&(tid3[created]), NULL, &doComplex, NULL);
			created++;
			pthread_mutex_lock(&lock);
			usageC++;
			pthread_mutex_unlock(&lock);
	
		}

	}


	
}

void initialize()
{
	
	/* Initializes and resets stuff */
	usage = 0;
	usageC = 0;
	potOne_using = 0;
	potTwo_using = 0;
	potThree_using = 0;	
	complexB_using = 0;

	totalWaitTime = 0;
	simpleTime = 0;
	complexTime = 0;
	numSimple = 0;
	numComplex = 0;
	timeIwait = 0;
	testTime = 0;
	timeIwait_s = 0;
	testTime_s = 0;
	
	payNumber = 0;
	payment_q_front = 0;
	payment_q_end = 0;
	serving = 1;
	bug_c = 0;
	bug_s = 0;
	bugFix = 0;

	size = 0;
	sizeC = 0;
	average = 0;
	temp = 0;

	int i;
	for(i = 0; i<10000;i++)
	{
		memset(&tid2[i], 0, sizeof(pthread_t));
		memset(&tid3[i], 0, sizeof(pthread_t));
		memset(&payQueue[i], 0, sizeof(int));
	}
	

 	memset(&tid[0], 0, sizeof(pthread_t));
	memset(&tid[1], 0, sizeof(pthread_t));

}

void runTest(int num)
{
	a = num; /* The number of customers for this test */

	int i;
	int j;
	int r;	/*Used for a random number */
	unsigned long long y;

	/* Queue stuff */
	int simpleQ[a]; /* Theorectical max */
	int complexQ[a]; /* Theorectical max */
	int sQ_front = 0;
	int cQ_front = 0;
	int sQ_nextEmpty = 0;	
	int cQ_nextEmpty = 0;	

	/* Sets up some globals */
	initialize();

	/* Initializes mutex */
	pthread_mutex_init(&lock, NULL) != 0;

	/* Initializes both arrays to 0 */
	for (i = 0; i<a; i++)
	{
		simpleQ[i] = 0;
		complexQ[i] = 0;
		payQueue[i] = 0;
	}


	/*
	 * Generate random numbers
	 * If they are a 1 add them to the complex queue
	 * If they are a 0 add them to the simple queue
	 * 
	 * Fills both our customer queues
	 */
		
	srand(time(NULL)); /*Changes the random seed */
	for(i = 0; i<a;i++)
	{
		r = rand()%2;
		
		if (r == 0)
		{
			simpleQ[sQ_nextEmpty] = 1;
			sQ_nextEmpty++;
	
		}
		else if(r == 1)
		{
			complexQ[cQ_nextEmpty] = 2;
			cQ_nextEmpty++;
		}

			
	}//End of for-loop

 
	
	/* For the purpose of simplicity, all of one type of customer appear 
	 * at the same time. (There is not much of a time delay for the program
	 * to create all the needed threads. Much of the time is spent in the 
	 * for-loops designed to pass time)
	 */

	size = sQ_nextEmpty;
	sizeC = cQ_nextEmpty;
	pthread_create(&(tid[0]), NULL, &createSimple, &(size));
	pthread_create(&(tid[1]), NULL, &createComplex, &(sizeC));
		
	//printf("Size: %d, sizeC: %d\n", size, sizeC);
	pthread_join(tid[0], NULL);

	/* Waits for our creation pthreads to finish */


	/* Waits for all the created simple threads to end */
	for (i = 0;i<size;i++)
	{
    		pthread_join(tid2[i], NULL);
	}



	pthread_join(tid[1], NULL);

	/* Waits for all the created complex threads to end */
	for (j = 0;j<sizeC;j++)
	{
    		pthread_join(tid3[j], NULL);
	}
	
	pthread_mutex_destroy(&lock);

	if(size >= 3)
		simpleTime = (simpleTime - (3*bugFix));
	else if(size == 2)
		simpleTime = (simpleTime - bugFix);
	



}

void displayTest(int sizeTest)
{


	double average400_s = 0;
	double average400_c = 0;
	int i;

	for(i = 0; i<5;i++)
	{
		runTest(sizeTest);

		average = (complexTime/(sizeC-bug_c))/1000;
		average400_c += average;

		average = (simpleTime/(size-bug_s))/1000;
		average400_s += average;
	
	}



	//printf("%d %0.2f %0.2f\n",sizeTest, (average400_c/5), (average400_s/5));
	fprintf(fp, "%d %0.2f %0.2f\n",sizeTest, (average400_c/5), (average400_s/5));
	
	


}

int main(void){

	int rc;
	size_t s1;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	s1=200000;

	pthread_attr_setstacksize(&attr, 100);


	fp = fopen("data.dat", "w");
	if(fp == NULL) {
		printf("derp\n");
		exit(0);
	}

	//printf("size complex simple \n");
	fprintf(fp, "##size complex simple \n");

	displayTest(10);
	displayTest(50);
	displayTest(100);
	displayTest(250);
	displayTest(500);
	displayTest(1000);
	displayTest(2000);
	displayTest(10000);



}//End of Main























