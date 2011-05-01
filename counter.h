#ifndef __COUNTER_H__
#define __COUNTER_H__

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/* counter data structure */ 
struct counter {
   long long counter;            /* to store counter */
   pthread_mutex_t lock; 
};

/* counter value */
struct instruction {
   struct counter *counter;      /* pointer to counter */
   int repetitions;              /* number of repetitions  */
   void (*work_fn)(long long *); /* function pointer to work function */
	
};

#endif

