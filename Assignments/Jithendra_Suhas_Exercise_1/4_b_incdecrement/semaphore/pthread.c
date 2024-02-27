/*******************************************************************************
 * Copyright (C) 2023 by Jithendra and Suhas
 *
 * Redistribution, modification, or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Jithendra, Suhas and the University of Colorado are not liable for
 * any misuse of this material.
 * ****************************************************************************/
/**
 * @file pthread.c
 * @brief Multithreaded increment and decrement operations using semaphores.
 *
 * This program demonstrates multithreaded increment and decrement operations
 * using POSIX threads and semaphores for synchronization. Two threads are created:
 * incThread and decThread, each responsible for performing either increment or
 * decrement operations on a global variable gsum.
 *
 * @author Jithendra and Suhas
 * @date February 10, 2024
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <unistd.h>
#include <semaphore.h>

#define COUNT  (1000)
#define NUM_THREADS (2)

typedef struct
{
    int threadIdx;
} threadParams_t;

pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];

// Semaphores
sem_t sem_inc, sem_dec;

// Unsafe global
int gsum=0;

/**
 * @brief Increment thread function.
 *
 * This function performs the increment operation on the global variable gsum
 * using semaphores for synchronization.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(&sem_inc); // Wait for the increment semaphore
        gsum = gsum + i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(&sem_dec); // Release the decrement semaphore
    }
}

/**
 * @brief Decrement thread function.
 *
 * This function performs the decrement operation on the global variable gsum
 * using semaphores for synchronization.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(&sem_dec); // Wait for the decrement semaphore
        gsum = gsum - i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(&sem_inc); // Release the increment semaphore
    }
}

/**
 * @brief Main function to control the execution of the program.
 *
 * This function initializes semaphores, creates threads, and waits for them to complete.
 *
 * @return 0 on successful execution.
 */
int main (int argc, char *argv[])
{
   int rc;
   int i=0;

   // Initialize semaphores
   sem_init(&sem_inc, 0, 1); // Initialize the increment semaphore to 1
   sem_init(&sem_dec, 0, 0); // Initialize the decrement semaphore to 0

   // Create increment and decrement thread
    for(i=0; i<NUM_THREADS; i++){
       threadParams[i].threadIdx = i;
       pthread_create(&threads[i], (void*)0, (i%2 ? decThread : incThread), (void *)&(threadParams[i]));
    }

   // Wait for threads to complete
   for(i=0; i<NUM_THREADS; i++)
       pthread_join(threads[i], NULL);

   // Destroy semaphores
   sem_destroy(&sem_inc);
   sem_destroy(&sem_dec);

   printf("TEST COMPLETE\n");

   return 0;
}