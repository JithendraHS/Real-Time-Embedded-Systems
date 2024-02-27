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
 * @file process2.c
 * @brief This program releases a binary semaphore for synchronization.
 * @author Jithendra H S
 * @date  02-25-2024
 */

#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <syslog.h>

#define BINARY_SEMAPHORE ("bin_sem")

int main() {
    // Open or create the binary semaphore
    sem_t *binary_semaphore = sem_open(BINARY_SEMAPHORE, O_CREAT, 0777, 0);

    // Display a message indicating that Process 2 is releasing the binary semaphore
    printf("Process 2 releasing the binary_semaphore\n");
    syslog(LOG_DEBUG, "Process 2 started and waiting for binary_semaphore");

    // Release the binary semaphore
    sem_post(binary_semaphore);

    return 0;
}
