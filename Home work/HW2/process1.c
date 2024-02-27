/*******************************************************************************
 * Copyright (C) 2023 by Jithendra
 *
 * Redistribution, modification, or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Jithendra and the University of Colorado are not liable for
 * any misuse of this material.
 * ****************************************************************************/
/**
 * @file process1.c
 * @brief This program demonstrates the usage of a binary semaphore for synchronization.
 * @author Jithendra H S
 * @date 02-25-2024
 */

#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <syslog.h>

#define BINARY_SEMAPHORE ("bin_sem")

int main() {
    // Open or create a binary semaphore with initial value 0
    sem_t *binary_semaphore = sem_open(BINARY_SEMAPHORE, O_CREAT, 0777, 0);

    // Display a message indicating that Process 1 has started and is waiting for the binary semaphore
    printf("Process 1 started and waiting for binary_semaphore\n");
    syslog(LOG_INFO, "Process 1 started and waiting for binary_semaphore");

    // Wait for the binary semaphore
    sem_wait(binary_semaphore);

    // Display a message indicating that Process 1 has completed after receiving the binary semaphore
    printf("Process 1 completed after receiving binary_semaphore\n");
    syslog(LOG_INFO, "Process 1 completed after receiving binary_semaphore");

   sem_close(binary_semaphore);
    return 0;
}
