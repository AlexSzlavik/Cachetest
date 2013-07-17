#ifndef _ZIPF_H_
#define _ZIPF_H_

#include <assert.h>             // Needed for assert() macro
#include <stdio.h>              // Needed for printf()
#include <stdlib.h>             // Needed for exit() and ato*()
#include <math.h>               // Needed for pow()
/*
 * zipf library header file
 * Define the API to the zipf RNG
 */

// Returns a Zipf random variable
extern int      zipf(double alpha, int n);  

// Jain's RNG
double   rand_val(int seed);         

//Initialize the generator
double init_zipf(int seed);

#endif
