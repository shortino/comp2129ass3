#include "counter.h"

/* ============================================================================
 * File-global variables
 * ========================================================================== */
static int ncounters = 0;
static struct counter *counters = NULL;

static int nthreads = 0;
static int *ninstructions = NULL;
static struct instruction **instructions = NULL;

pthread_mutex_t flock;
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
	*n = *n+*n;
}


/* ============================================================================
 * Helper functions
 * ========================================================================== */
int get_numbers(void){
	int scan;
	if ((scan = scanf("%i", &ncounters)) && ncounters > 0) {
		if ((scan = scanf("%i", &nthreads)) && nthreads > 0) {
			return 1;								// returns true if scan gets 1 input 
		}
	}
	return 0;
}

void set_counters(void) {
	int i;
	int a = 0;
	/* allocate an array of size ncounters */
	counters = malloc(ncounters * sizeof(struct counter));
	/* initalise the counters to 0 */
	for (i = 0; i < ncounters; ++i){
		counters[i].counter = a;
	}
}

void (*get_function(const char f_id))(long long *){
	if (f_id == 'D') {
		return &decrement;
	}
	else if (f_id == 'I') {
		return &increment;
	}

	return &mult2; //assumes no mal-formed input will be tested
}

/* gets individual instruction sequence lines and sets the values - called by get_instructions*/
int get_instruction_sequence(int threadid, int instructionid) {
	int scan, i = threadid, j = instructionid;
	int tempCounterid, tempIncrement;
	char functionid;
	if ((scan = scanf("%i %c %i", &tempCounterid, &functionid, &tempIncrement))) {
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
	ninstructions = (int *)malloc(sizeof(int *));
	/* alocate an array of pointers length nthreads first*/
	instructions = malloc(nthreads * sizeof(struct instruction *));
	/* for each pointer allocate dynamic array of instructions*/
	for (i = 0; i < nthreads; ++i) {
		/* get number of instructions */
		scan = scanf("%i", &temp);
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
/* my own printout method for the 2darry to check values */
void print_out_2darray(void) {
	int i, j;
	for (i = 0; i < nthreads; ++i) {
		for (j = 0; j < ninstructions[i]; ++j) {
			printf("incrementer for instruction %i %i is %i\n", i, j, instructions[i][j].repetitions);
		}
	}
}


void print_out_counters(void) {
	int i;
	//struct counter*j;
	for (i = 0; i < ncounters; ++i) {
		printf("%d\n", (int)counters[i].counter);
	}
	/*
	   for (i = 0; i < nthreads; ++i) {
	   for (j = 0; j < ninstructions[i]; ++j) {
	   printf("%i\n", instructions[i][j].counter->counter);
	   }
	   }
	   */
}

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

	/* free the pthreads and mutex */
	//doesnt apply

	return 1;
}


/* ============================================================================
 * Thread function
 * ========================================================================== */
static void *
worker_thread(void *arg) {
	int i, j;
	long id =  *(int *) arg;
	/* work through all instructions for the thread */
	//printf("ninstructions %i\n", ninstructions[id]);
	for (j = 0; j < ninstructions[id]; ++j) {
		// for the repetitions
		//printf("calling lock id j %d %d\n", id, j);

		/* mutex for the calling of each function */
		pthread_mutex_lock(&flock);
		//pthread_mutex_lock(&instructions[id][j].counter->lock); //CHECK SLIDE 22!!
		for (i = 0; i < instructions[id][j].repetitions; ++i) {
			// call work_fn with that counter
			//printf("repetitions %i\n", i);
			instructions[id][j].work_fn(&instructions[id][j].counter->counter);
		}

		//pthread_mutex_unlock(&instructions[id][j].counter->lock);
		pthread_mutex_unlock(&flock);
	}

	return NULL;
}


/* ============================================================================
 * Main function
 * ========================================================================== */
int
main(void) {
	int i, checkt;
	pthread_t counter_thread[nthreads];
	/* read in number of counters */
	if (get_numbers()) {
		//continue if true
		set_counters();				// allocate the array of counters
		if (get_instructions()) {
			// create the threads executing working_thread
			for (i = 0; i < nthreads; ++i) {
				void * t = (void *)&i;													//pass in address of int i casted a void ptr 
				checkt = pthread_create(&counter_thread[i], NULL, worker_thread, t);
				if (checkt){
					printf("error creating thread %d\n", i);
				}
			}
			// execute the instruction per thread
			// enfore with mutex
			/* join all threads */
			for (i = 0; i < nthreads; ++i) {
				pthread_join(counter_thread[i], NULL);
			}

		}
		else {
			printf("error\n");		// error for get_instructions
			exit(0);
		}
	}
	else  {
		printf("error\n");			// error in get_numbers function
		exit(0);
	}
	// display counter states	
	print_out_counters();				
	// free memory
	if (!free_memory()) {
		printf("problem freeing memory\n");
	}
	
	//print_out_2darray();
	// num of counters
	// num of threads == num of instruction sequences 
	// instruction sequence == num of instructions 
	//		num of instructions
	//		counter - work function - repition
	//


	//dynamically allocated
	// *counters[ncounter] -> counter
	// *ninstructions[nthreads] -> instruction x`

	return 0;
}
