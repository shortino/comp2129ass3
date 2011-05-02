#include "counter.h"

/* ============================================================================
 * File-global variables
 * ========================================================================== */
static int ncounters = 0;
static struct counter *counters = NULL;

static int nthreads = 0;
static int *ninstructions = NULL;
static struct instruction **instructions = NULL;

pthread_mutex_t flock; // DELETE LATER
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
	counters = (struct counter *)malloc(ncounters * sizeof(struct counter));
	/* initalise the counters to 0 and the mutex lock*/
	for (i = 0; i < ncounters; ++i){
		counters[i].counter = a;
		//counters[i].lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		//pthread_mutex_init(&counters[i].lock, NULL);
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
	ninstructions = (int *)malloc(nthreads * sizeof(int *));
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



	return 1;
}


/* ============================================================================
 * Thread function
 * ========================================================================== */
static void *
worker_thread(void *arg) {
	int i = 0, 
		j = 0;
	long id =  *(int *) arg;
	int thread_ninstructions = ninstructions[id];
	//int thread_repetitions = 

	
	/* work through all instructions for that thread */
	for (; j < thread_ninstructions; ++j) {
		pthread_mutex_lock(&instructions[id][j].counter->lock); 
		printf("threadid and column %d %d\n", id, j);
		printf("number of instructions for thread %d\n", ninstructions[id]);
		
		/* for each repetition call the function */
		for (i; i < instructions[id][j].repetitions; ++i) {
			printf("locking %d %d\n", *(int *) arg, j);
			//pthread_mutex_lock(&instructions[id][j].counter->lock); 
			//pthread_mutex_lock(&flock);
			instructions[id][j].work_fn(&instructions[id][j].counter->counter);
			//pthread_mutex_unlock(&instructions[id][j].counter->lock);
		}
		
		pthread_mutex_unlock(&instructions[id][j].counter->lock);
	//pthread_mutex_unlock(&flock);
	}

	return NULL;
}


/* ============================================================================
 * Main function
 * ========================================================================== */
int
main(void) {
	int i, j, b, k, checkt;
	int *taskid[nthreads];							//args for each thread
	//pthread_mutex_lock(&flock);
	//pthread_t counter_thread[nthreads];
	
	/* read in number of counters */
	if (get_numbers()) {
		set_counters();				// allocate the array of counters
		if (get_instructions()) {
			
			/* initalise counter locks */
			for (b = 0; b < ncounters; ++b) {
				//counters[b].lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
				pthread_mutex_init(&counters[b].lock, NULL); //TEMP
			}
			/* declare the num of threads */
			pthread_t counter_thread[nthreads];
			
			/* create the threads executing working_thread */
			for (i = 0; i < nthreads; ++i) {
				/* allocate memory for the taskid */
				taskid[i] = (int *)malloc(sizeof(int));										// create space for each arg
				*taskid[i] = i;																// assign the incrementer as the arg
				printf("calling create with arg %d\n", i);
				pthread_create(&counter_thread[i], NULL, worker_thread, (void *) taskid[i]);
				checkt = pthread_create(&counter_thread[i], NULL, worker_thread, (void *) taskid[i]);
				/* error checking */
				if (checkt){
					printf("error creating thread %d\n", i);
					exit(1);
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
		else {
			printf("error\n");		// error for get_instructions
			exit(0);
		}
	}
	else  {
		printf("error\n");			// error in get_numbers function
		exit(0);
	}
	/* display counter states */	
	print_out_counters();				
	/* free memory from global and local allocations */
	if (!free_memory()) {
		printf("problem freeing memory\n");
	}	
	for (k = 0; k < nthreads; ++k) {
		//free(taskid[k]);			// free thread arguments ints
	}

	return 0;
}
