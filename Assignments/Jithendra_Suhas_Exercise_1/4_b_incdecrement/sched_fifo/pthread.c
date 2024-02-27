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
 * @brief Multithreaded increment and decrement operations with real-time scheduling.
 *
 * This program demonstrates multithreaded increment and decrement operations
 * using POSIX threads with real-time scheduling policies. It defines two threads:
 * incThread and decThread, each responsible for performing either increment or
 * decrement operations on a global variable gsum.
 *
 * @author Jithendra and Suhas
 * @date February 10, 2024
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sched.h>

#define COUNT 1000
#define NUM_THREADS (2)
#define NUM_CPUS (1)

// Unsafe global
int gsum=0;

typedef struct {
    int threadIdx;
} threadParams_t;

pthread_t threads[2];
threadParams_t threadParams[2];
int rt_max_prio, rt_min_prio;

// Schedule param
struct sched_param main_param;

/**
 * @brief Increment thread function.
 *
 * This function performs the increment operation on the global variable gsum.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *incThread(void *threadp) {
  int i;
  int policy = SCHED_FIFO;
  struct sched_param param;
  threadParams_t *threadParams = (threadParams_t *)threadp;

  /* scheduling parameters of target thread */
  int ret = pthread_getschedparam (pthread_self(), &policy, &param);

  for (i = 0; i < COUNT; i++) {
    gsum = gsum + i;
    printf("Increment thread idx=%d, priority = %d, gsum=%d\n", threadParams->threadIdx, param.sched_priority,  gsum);
  }
}

/**
 * @brief Decrement thread function.
 *
 * This function performs the decrement operation on the global variable gsum.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *decThread(void *threadp) {
  int i;
  int policy = SCHED_FIFO;
  struct sched_param param;
  threadParams_t *threadParams = (threadParams_t *)threadp;

  /* scheduling parameters of target thread */
  int ret = pthread_getschedparam (pthread_self(), &policy, &param);

  for (i = 0; i < COUNT; i++) {
    gsum = gsum - i;
    printf("Decrement thread idx=%d, priority = %d, gsum=%d\n", threadParams->threadIdx, param.sched_priority,  gsum);
  }
}

/**
 * @brief Prints the scheduling policy of the current process.
 *
 * This function retrieves the scheduling policy of the current process
 * and prints the corresponding string representation.
 *
 * @return None.
 */
void print_scheduler(void) {
  int schedType;

  schedType = sched_getscheduler(getpid());

  switch (schedType) {
  case SCHED_FIFO:
    printf("Pthread Policy is SCHED_FIFO\n");
    break;
  case SCHED_OTHER:
    printf("Pthread Policy is SCHED_OTHER\n");
    break;
  case SCHED_RR:
    printf("Pthread Policy is SCHED_RR\n");
    break;
  default:
    printf("Pthread Policy is UNKNOWN\n");
  }
}

/**
 * @brief Main function to control the execution of the program.
 *
 * This function initializes parameters, creates threads, sets scheduling policies,
 * and waits for threads to complete.
 *
 * @return 0 on successful execution.
 */
int main(int argc, char *argv[]) {
    int rc;
    int i = 0;
    struct sched_param rt_param;
    int policy = SCHED_FIFO;
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for(i=0; i < NUM_CPUS; i++)
       CPU_SET(i, &cpuset);
    print_scheduler();

    // Get scheduling parameters of main process
    rc = sched_getparam(getpid(), &main_param);
    rt_max_prio = sched_get_priority_max(SCHED_FIFO);
    rt_min_prio = sched_get_priority_min(SCHED_FIFO);
    printf("rt_max_prio=%d\n", rt_max_prio);
    printf("rt_min_prio=%d\n", rt_min_prio);

    // Set main process priority to maximum
    main_param.sched_priority = rt_max_prio;
    rc = sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if (rc < 0)
      perror("main_param");

    print_scheduler();

    // Create threads
    for (i = 0; i < NUM_THREADS; i++) {
        threadParams[i].threadIdx = i;

        // Create thread with FIFO scheduling policy
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        rc=pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr, policy);
        rc=pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
        rt_param.sched_priority = rt_max_prio - i -  1;
        pthread_attr_setschedparam(&attr, &rt_param);

        // Decide which thread function to execute based on thread index
        rc = pthread_create(&threads[i], &attr, (i % 2 ? decThread : incThread), (void *)&

(threadParams[i]));
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    // Wait for threads to complete
    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    printf("TEST COMPLETE\n");

    return 0;
}
