#include "counter.h"

/* ============================================================================
 * File-global variables
 * ========================================================================== */
static int ncounters = 0;
static struct counter *counters = NULL;

static int nthreads = 0;
static int *ninstructions = NULL;
static struct instruction **instructions = NULL;

static int *taskid = NULL;						// used for passing args into threads 
static pthread_t *counter_thread = NULL;		// thread declaration

/* ============================================================================
 * Operations
 * ========================================================================== */
static void
decrement(long long *n) {
	*n = *n-1;	
}

static void
increment(long long *n) {
	*n = *n+1;
}

static void
mult2(long long *n) {
	(*n) *= 2;
}


/* ============================================================================
 * Helper functions
 * ========================================================================== */

/* free all allocated memory, called from either error() or main() */
int free_memory(void) {
	int i;
	/* free the 2d array elements then the actual array */
	for (i = 0; i < nthreads; ++i) {
		free(instructions[i]);
	}
	free(instructions);

	/* free the instruction sequence array */
	free(ninstructions);

	/* free the counter array */
	free(counters);

	/* free thread arguments */
	free(taskid);
	
	/* free threads */
	free(counter_thread);
	
	/* set arrays to NULL */
	counter_thread = NULL;
	counters = NULL;
	ninstructions = NULL;
	instructions = NULL;

	return 1;
}

/* error function that frees memory and exits */
void error(void) {
	printf("error\n");
	/* free memory from global allocations */
	if (!free_memory()) {
		printf("problem freeing memory\n");
	}	
	// exit program
	exit(0);
}

/* gets the ncounters and nthreads first */
int get_numbers(void){
	int scan;
	if ((scan = scanf("%i", &ncounters)) && ncounters > 0) {
		if ((scan = scanf("%i", &nthreads)) && nthreads > 0) {
			return 1;	// returns true if scan gets correct input
		}
	}
	return 0;
}

/* allocate the array of counters and initalise to zero */
void set_counters(void) {
	int i;
	int a = 0;
	/* allocate an array of size ncounters */
	counters = (struct counter *)malloc(ncounters * sizeof(struct counter));
	/* initalise the counters to 0 and the mutex locks */
	for (i = 0; i < ncounters; ++i){
		counters[i].counter = a;
		pthread_mutex_init(&counters[i].lock, NULL); 
	}
}

/* returns the matching function */
void (*get_function(const char f_id))(long long *){
	switch (f_id) {
		case 'I':
			return &increment;
		case 'D':
			return &decrement;
		case '2':
			return &mult2;
		default:
			error();
	}
	return NULL;
}

/* gets individual instruction sequence lines and sets the values, called by get_instructions*/
int get_instruction_sequence(int threadid, int instructionid) {
	int scan, i = threadid, j = instructionid;
	int tempCounterid, tempIncrement;
	char functionid;
	if ((scan = scanf("%i %c %i", &tempCounterid, &functionid, &tempIncrement))) {
		/* check repetitions is non-negative */
		if(tempIncrement <= 0) {
			return 0;
		}
		/* set the instructions values for each squence */
		instructions[i][j].counter = &counters[tempCounterid];		// set the counter to equal the ptr to the counter in the counter array
		instructions[i][j].repetitions = tempIncrement;
		instructions[i][j].work_fn = get_function(functionid); 
		return 1;
	}
	return 0;
}

/* allocates memory for the 2d array and sets the instructions*/
int get_instructions(void){
	int i, j, scan;
	int temp; 
	/* alocate an array to hold instruction sequence lengths for each thread*/
	ninstructions = (int *) malloc(nthreads * sizeof(int));
	/* alocate an array of pointers length nthreads first*/
	instructions = malloc(nthreads * sizeof(struct instruction *));
	/* for each pointer allocate dynamic array of instructions*/
	for (i = 0; i < nthreads; ++i) {
		/* get number of instructions */
		scan = scanf("%i", &temp);  // check for valid number
		ninstructions[i] = temp;
		/* set the variable row 'length'/columns of the 2D array */
		instructions[i] = malloc(ninstructions[i] * sizeof(struct instruction));
		/* grab the instruction sequence by calling the grab method */
		for (j = 0; j < temp; ++j) {
			if (get_instruction_sequence(i, j)) {
				continue;
			}
			else
				return 0;
		}

	}
	return 1;
}


/* my own private printout method for the 2darry to check values */
/*void print_out_2darray(void) {
	int i, j;
	for (i = 0; i < nthreads; ++i) {
		for (j = 0; j < ninstructions[i]; ++j) {
			printf("struct counter %i\n int repetitions %d\n", (int)instructions[i][j].counter->counter, instructions[i][j].repetitions);
			printf("trying lock\n");
			pthread_mutex_lock(&instructions[i][j].counter->lock); 
			pthread_mutex_unlock(&instructions[i][j].counter->lock); 
		}
	}
}*/

void print_out_counters(void) {
	int i;
	//struct counter*j;
	for (i = 0; i < ncounters; ++i) {
		printf("%d\n", (int)counters[i].counter);
	}
}


/* ============================================================================
 * Thread function
 * ========================================================================== */
static void *
worker_thread(void *arg) {
	
	long i = 0, 
		 j = 0;
	long id =  *(int *) arg;
	int *thread_ninstructions = &ninstructions[id];

	/* work through all instructions for that thread */
	for (j = 0; j < *thread_ninstructions; ++j) {
		
		/* set the number of repetitions with j */
		int *thread_repetitions = &instructions[id][j].repetitions;
		
		/* lock the counter mutex to perform the counter instructions */
		pthread_mutex_lock(&instructions[id][j].counter->lock); 		
		
		/* for each repetition call the function */
		for (i = 0; i < *thread_repetitions; ++i) {
			instructions[id][j].work_fn(&instructions[id][j].counter->counter);
		}
		pthread_mutex_unlock(&instructions[id][j].counter->lock);
	}
	return NULL;
}

/* ============================================================================
 * Main function
 * ========================================================================== */
int
main(void) {
	int i, j, checkt;
	/* read in number of counters */
	if (get_numbers()) {
		set_counters();				// allocate the array of counters
		if (get_instructions()) {
			
			/*allocate memory for taskid and threads*/
			taskid = (int *)malloc(nthreads * sizeof(int));		
			counter_thread = calloc(nthreads, sizeof(pthread_t));
			/* create the threads executing working_thread */
			for (i = 0; i < nthreads; ++i) {				
				taskid[i] = i;																// assign the incrementer as the arg
				checkt = pthread_create(&counter_thread[i], NULL, worker_thread, (void *) &taskid[i]);
				
				/* error checking */
				if (checkt){
					printf("error creating thread %d\n", i);
					exit(0);
				}
			}
			/* join all threads */	
			for (j = 0; j < nthreads; ++j) {
				checkt = pthread_join(counter_thread[j], NULL);
				if (checkt) {
					printf("error return code from thread %d\n", checkt);
				}
			}
		}
		else
			error(); // error for get_instructions
	}
	else
		error(); // error in get_numbers function

	/* display counter states */	
	print_out_counters();	

	/* free memory from global and local allocations */
	if (!free_memory()) {
		printf("problem freeing memory\n");
	}	
	return 0;
}
