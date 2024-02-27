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
 * @file pthreads.c
 * @brief Multithreaded Fibonacci sequence computation with real-time scheduling.
 *
 * This program demonstrates multithreaded Fibonacci sequence computation
 * using POSIX threads with real-time scheduling policies. It defines three
 * threads: sequencer, fib10, and fib20, each responsible for specific tasks
 * in the Fibonacci sequence calculation.
 *
 * @authors Jithendra and Suhas
 * @date February 10, 2024
 */
#define _GNU_SOURCE
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <syslog.h>

#define NUM_THREADS (3)
#define NUM_CPUS (1)
#define ITER (3)

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_MICROSEC (1000)
#define DELAY_TICKS (1)
#define ERROR (-1)
#define OK (0)
#define TEST_CYCLES (10000000)
#define FIB_10_CI (10)
#define FIB_20_CI (20)

typedef struct
{
    int threadIdx;
} threadParams_t;

// POSIX thread declarations and scheduling attributes
//
pthread_t threads[NUM_THREADS];
threadParams_t threadParams[NUM_THREADS];
pthread_attr_t rt_sched_attr[NUM_THREADS];
int rt_max_prio, rt_min_prio;
struct sched_param rt_param[NUM_THREADS];
struct sched_param main_param;
pthread_attr_t main_attr;
pid_t mainpid;

sem_t sem_fib10, sem_fib20;
int fib10Cnt=0, fib20Cnt=0;
struct timespec start_time = {0, 0};

unsigned int idx = 0, jdx = 1;
unsigned int seqIterations = 47;
volatile unsigned int fib = 0, fib0 = 0, fib1 = 1;
volatile int abort_test = 0;

// Calculate Fibonacci sequence and store it in fib
#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }                                   \

/**
 * @brief Calculates the time difference between two timespec structures.
 *
 * This function calculates the time difference between two timespec structures
 * representing start and stop times, and stores the result in another timespec
 * structure.
 *
 * @param stop Pointer to the timespec structure representing the stop time.
 * @param start Pointer to the timespec structure representing the start time.
 * @param delta_t Pointer to the timespec structure where the time difference will be stored.
 * @return Always returns 1 to indicate success.
 */
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
    // Calculate the difference in seconds and nanoseconds between stop and start times
    int dt_sec = stop->tv_sec - start->tv_sec;
    int dt_nsec = stop->tv_nsec - start->tv_nsec;

    // Adjust the time difference if necessary to ensure it's valid
    if (dt_sec >= 0)
    {
        if (dt_nsec >= 0)
        {
            // If both seconds and nanoseconds are positive or zero, no adjustment is needed
            delta_t->tv_sec = dt_sec;
            delta_t->tv_nsec = dt_nsec;
        }
        else
        {
            // If nanoseconds are negative, adjust the time difference accordingly
            delta_t->tv_sec = dt_sec - 1;
            delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec; // Add one second's worth of nanoseconds
        }
    }
    else
    {
        // If seconds are negative, adjust the time difference accordingly
        if (dt_nsec >= 0)
        {
            delta_t->tv_sec = dt_sec;
            delta_t->tv_nsec = dt_nsec;
        }
        else
        {
            // If both seconds and nanoseconds are negative, adjust the time difference accordingly
            delta_t->tv_sec = dt_sec - 1;
            delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec; // Add one second's worth of nanoseconds
        }
    }

    // Return 1 to indicate success
    return 1;
}

/**
 * @brief Thread function to calculate Fibonacci sequence for FIB_10.
 *
 * This function estimates the number of iterations required to compute
 * the Fibonacci sequence for FIB_10 and performs the computation.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *fib10(void *threadp)
{
    // Obtain the thread ID
    pthread_t fib10_t = pthread_self();

    // Define real-time scheduling parameters
    struct sched_param rt_param;

    // Define real-time scheduling policy
    int policy = SCHED_FIFO;

    // Get the thread's scheduling parameters
    pthread_getschedparam(fib10_t , &policy, &rt_param);

    // Define finish time of the thread
    struct timespec finish_time = {0, 0};

    // Define time difference for the thread
    struct timespec thread_dt = {0, 0};

    // Define thread parameters
    threadParams_t *threadParams = (threadParams_t *)threadp;

    // Define start time for testing
    struct timespec test_start_time = {0, 0};

    // Define end time for testing
    struct timespec test_end_time = {0, 0};

    // Define time difference for testing
    struct timespec test_delta_time = {0, 0};

    // Print estimation message
    printf("Estimating FIB_10 required iterations\n");

    // Get start time for testing
    clock_gettime(CLOCK_REALTIME, &test_start_time);

    // Test Fibonacci sequence
    FIB_TEST(seqIterations, TEST_CYCLES);

    // Get end time for testing
    clock_gettime(CLOCK_REALTIME, &test_end_time);

    // Calculate testing time difference
    delta_t(&test_end_time, &test_start_time, &test_delta_time);

    // Estimate Fibonacci 10 iterations
    int fib10_iter_count = ((float)FIB_10_CI/(test_delta_time.tv_nsec / NSEC_PER_MSEC)) * TEST_CYCLES;

    // Execute Fibonacci calculations until abort_test is set
    while(!abort_test){
        // Wait for semaphore signal
        sem_wait(&sem_fib10);

        // Break loop if abort_test is true
        if(abort_test) break;

        // Get finish time
        clock_gettime(CLOCK_REALTIME, &finish_time);

        // Calculate thread time difference
        delta_t(&finish_time, &start_time, &thread_dt);

        // Print thread start message
        printf("FIB_10           %d            started        %ld sec, %ld msec \n", rt_param.sched_priority,  thread_dt.tv_sec, (thread_dt.tv_nsec / NSEC_PER_MSEC));

        // Compute Fibonacci sequence
	      FIB_TEST(seqIterations, fib10_iter_count);

        // Increment Fibonacci 10 count
        fib10Cnt++;

        // Get finish time
        clock_gettime(CLOCK_REALTIME, &finish_time);

        // Calculate thread time difference
        delta_t(&finish_time, &start_time, &thread_dt);

        // Print thread completion message
        printf("FIB_10           %d           Completed       %ld sec, %ld msec \n", rt_param.sched_priority,  thread_dt.tv_sec, (thread_dt.tv_nsec / NSEC_PER_MSEC));

        // Log thread completion
        syslog (LOG_INFO, "Thread FIB_10 completed at %ld", (thread_dt.tv_nsec / NSEC_PER_MSEC));
    }

    // Exit thread
    pthread_exit(NULL);
}


/**
 * @brief Thread function to calculate Fibonacci sequence for FIB_20.
 *
 * This function estimates the number of iterations required to compute
 * the Fibonacci sequence for FIB_20 and performs the computation.
 *
 * @param threadp Pointer to thread parameters.
 * @return None.
 */
void *fib20(void *threadp)
{
    // Obtain the thread ID
    pthread_t fib20_t = pthread_self();

    // Define real-time scheduling parameters
    struct sched_param rt_param;

    // Define real-time scheduling policy
    int policy = SCHED_FIFO;

    // Get the thread's scheduling parameters
    pthread_getschedparam(fib20_t , &policy, &rt_param);

    // Define finish time of the thread
    struct timespec finish_time = {0, 0};

    // Define time difference for the thread
    struct timespec thread_dt = {0, 0};

    // Define thread parameters
    threadParams_t *threadParams = (threadParams_t *)threadp;

    // Define start time for testing
    struct timespec test_start_time = {0, 0};

    // Define end time for testing
    struct timespec test_end_time = {0, 0};

    // Define time difference for testing
    struct timespec test_delta_time = {0, 0};

    // Print estimation message
    printf("Estimating FIB_20 required iterations\n");

    // Get start time for testing
    clock_gettime(CLOCK_REALTIME, &test_start_time);

    // Test Fibonacci sequence
    FIB_TEST(seqIterations, TEST_CYCLES);

    // Get end time for testing
    clock_gettime(CLOCK_REALTIME, &test_end_time);

    // Calculate testing time difference
    delta_t(&test_end_time, &test_start_time, &test_delta_time);

    // Estimate Fibonacci 20 iterations
    int fib20_iter_count = ((float)FIB_20_CI/(test_delta_time.tv_nsec / NSEC_PER_MSEC)) * TEST_CYCLES;

    // Execute Fibonacci calculations until abort_test is set
    while(!abort_test){
        // Wait for semaphore signal
        sem_wait(&sem_fib20);

        // Break loop if abort_test is true
        if(abort_test) break;

        // Get finish time
        clock_gettime(CLOCK_REALTIME, &finish_time);

        // Calculate thread time difference
        delta_t(&finish_time, &start_time, &thread_dt);

        // Print thread start message
        printf("FIB_20           %d            started        %ld sec, %ld msec \n", rt_param.sched_priority,  thread_dt.tv_sec, (thread_dt.tv_nsec / NSEC_PER_MSEC));

        // Compute Fibonacci sequence
	      FIB_TEST(seqIterations, fib20_iter_count);

        // Increment Fibonacci 20 count
        fib20Cnt++;

        // Get finish time
        clock_gettime(CLOCK_REALTIME, &finish_time);

        // Calculate thread time difference
        delta_t(&finish_time, &start_time, &thread_dt);

        // Print thread completion message
        printf("FIB_20           %d           Completed       %ld sec, %ld msec \n", rt_param.sched_priority,  thread_dt.tv_sec, (thread_dt.tv_nsec / NSEC_PER_MSEC));

        // Log thread completion
        syslog (LOG_INFO, "Thread FIB_20 completed at %ld", (thread_dt.tv_nsec / NSEC_PER_MSEC));
    }

    // Exit thread
    pthread_exit(NULL);
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

  // Get the scheduling policy of the current process
  schedType = sched_getscheduler(getpid());

  // Switch based on the scheduling policy type
  switch (schedType) {
  case SCHED_FIFO:
  // Print message for SCHED_FIFO
    printf("Pthread Policy is SCHED_FIFO\n");
    break;
  case SCHED_OTHER:
   // Print message for SCHED_OTHER
    printf("Pthread Policy is SCHED_OTHER\n");
    break;
  case SCHED_RR:
   // Print message for SCHED_RR
    printf("Pthread Policy is SCHED_RR\n");
    break;
  default:
   // Print message for unknown policy
    printf("Pthread Policy is UNKNOWN\n");
  }
}


/**
 * @brief Sequencer function to control the execution of Fibonacci threads.
 *
 * This function coordinates the execution of Fibonacci threads, controlling
 * the timing and frequency of their execution.
 *
 * @param threadp Pointer to thread parameters (unused).
 * @return None.
 */
void *sequencer(void* threadp){  
    int times = 0; // Counter for the number of iterations
    while(times < ITER){ // Iterate until the specified number of iterations is reached
        // Print header for trial
        printf("----------------------------------------------------------\n");
        printf("                   Trial %d\n", (times + 1));
        printf("----------------------------------------------------------\n");
        printf("THREAD        PRIORITY          STATUS          TIMESTAMP\n");
        printf("----------------------------------------------------------\n");

        // Get current timestamp
        clock_gettime(CLOCK_REALTIME, &start_time);

        // Log scheduling start
        syslog (LOG_INFO, "*************schedule started*************");

        // Signal Fibonacci threads to start execution
        sem_post(&sem_fib10);
        sem_post(&sem_fib20);

        // Sleep to control timing
        usleep(20000);

        // Signal Fibonacci threads to continue execution
        sem_post(&sem_fib10);

        // Sleep to control timing
        usleep(20000);

        // Signal Fibonacci threads to continue execution
        sem_post(&sem_fib10);

        // Sleep to control timing
        usleep(10000);

        // Signal Fibonacci threads to continue execution
        sem_post(&sem_fib20);

        // Sleep to control timing
        usleep(10000); 

        // Signal Fibonacci threads to continue execution
        sem_post(&sem_fib10);

        // Sleep to control timing
        usleep(20000);

        // Signal Fibonacci threads to continue execution
        sem_post(&sem_fib10);

        // Sleep to control timing
        usleep(20000);

        times++; // Increment trial counter

        // Print counts for fib10 and fib20 threads
        printf("----------------------------------------------------------\n");
        printf("fib10 ran count %d, fib20 ran count %d\n", fib10Cnt, fib20Cnt);
        printf("----------------------------------------------------------\n");
    }

    // Set abort_test flag to terminate threads
    abort_test = 1;

    // Signal Fibonacci threads to continue execution
    sem_post(&sem_fib10);
    sem_post(&sem_fib20);

    // Exit sequencer thread
    pthread_exit(NULL);
}

/**
 * @brief Main function to control the execution of the program.
 *
 * This function initializes parameters, creates threads, sets scheduling policies,
 * and waits for threads to complete.
 *
 * @return 0 on successful execution.
 */
int main(){
    
    int rc;
    int i = 0;
   // Define a CPU set and initialize it
   cpu_set_t cpuset;
   CPU_ZERO(&cpuset);

  // Loop through the CPUs and add them to the CPU set
  for(i = 0; i < NUM_CPUS; i++)
    CPU_SET(i, &cpuset);

    // Print start message
    printf("TEST STARTED\n");

    // Open system log
    openlog ("Fibonacci", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog (LOG_INFO, "TEST_STARTED");

    // Print current scheduler type
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

    // Print updated scheduler type
    print_scheduler();

    // Initialize semaphores
    sem_init(&sem_fib10, 0, 0);
    sem_init(&sem_fib20, 0, 0);

    // Define the thread routine function pointer type
    typedef void *(*Threadpointer)(void*);
    Threadpointer Threads[] = {sequencer, fib10, fib20};

    // Create threads for Fibonacci computations
    for (i = 0; i < NUM_THREADS; i++) {
        threadParams[i].threadIdx = i;
        // Create thread with FIFO scheduling policy
        pthread_attr_t attr;
        rc = pthread_attr_init(&attr);
        rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        rc = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        rc = pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpuset);
        rt_param[i].sched_priority = rt_max_prio - i - 1;
        pthread_attr_setschedparam(&attr, &rt_param[i]);

        // Decide thread function based on thread index
        rc = pthread_create(&threads[i], (void *)&attr, Threads[i], (void *)&(threadParams[i]));
        if (rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }

    // Wait for threads to complete
    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    // Destroy semaphores
    sem_destroy(&sem_fib10);
    sem_destroy(&sem_fib20);

    // Print completion message
    printf("TEST COMPLETE\n");

    // Close system log
    closelog ();

    return 0;
}
