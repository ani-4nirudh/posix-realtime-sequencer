/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file main.c
 * @brief Main function that creates thread objects and contains the function to
 * calculate the sum as per the thread index value
 *
 * @author Anirudh Singh
 * @date 11th August 2026
 */

#define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "app_config.h"
#include "set_attr_sched.h"
#include "sysinfo.h"

/**
 * Define the thread parameters
 */
typedef struct {
  int thread_idx;
} thread_params_t;

/**
 * Function to calculate sum
 */
void *sum_thread(void *thread_ptr) {
  int sum = 0;
  thread_params_t *thread_params = (thread_params_t *)(thread_ptr);
  int idx = thread_params->thread_idx;

  /**
   * calculate sum
   */
  for (int i = 0; i < idx; i++) {
    sum = sum + (i + 1);
  }

  /*
   * Get core
   */
  int core = sched_getcpu();

  /**
   * Print to syslog
   */
  syslog(LOG_INFO, "%s: Thread idx=%d, sum[1...%d]=%d Running on core : %d",
         COURSE_PREFIX, idx, idx, sum, core);

  return NULL;
}

int main(int argc, char *argv[]) {
  openlog("incdecthread", LOG_PID | LOG_CONS, LOG_DAEMON); // Open logging

  /**
   * Get system infog
   */
  struct utsname info;
  if (get_sys_info(&info) == -1) {
    printf("%s get_sys_info() failure!\n", COURSE_PREFIX);
    closelog();
    return -1;
  }

  /**
   * System info in syslog
   */
  syslog(LOG_INFO, "%s %s %s %s %s %s GNU/LINUX", COURSE_PREFIX, info.sysname,
         info.nodename, info.release, info.version, info.machine);

  /**
   * Multi-threading operation to calculate sum
   */
  pthread_t thread[NUM_THREADS]; // Create the array of thread objects
  thread_params_t thread_params[NUM_THREADS]; // Array of thread parameters

  /**
   * Creating thread attributes and setting priority
   */
  pthread_attr_t attr_t; // Create thread attributes
  int new_prio = 20;

  if (set_thread_attr_sched(&attr_t, new_prio) != 0) {
    fprintf(stderr, "set_thread_attr_sched() failure!");
    pthread_attr_destroy(&attr_t);
    closelog();
    return -1;
  }

  for (int i = 0; i < NUM_THREADS; i++) { // Initialise thread indices
    thread_params[i].thread_idx = i + 1;
  }

  // Execute the sum function on different threads
  for (int i = 0; i < NUM_THREADS; i++) {
    int ret = pthread_create(&thread[i], &attr_t, sum_thread,
                             (void *)&(thread_params[i]));
    if (ret != 0) {
      fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
      fprintf(stderr, "Run executable using SUDO\n");
      pthread_attr_destroy(&attr_t);
      closelog();
      return ret;
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) { // Join the threads
    int ret = pthread_join(thread[i], NULL);
    if (ret != 0) {
      fprintf(stderr, "pthread_join error: %s\n", strerror(ret));
      pthread_attr_destroy(&attr_t);
      closelog();
      return ret;
    }
  }

  closelog(); // Close logging and exit
  return 0;
}
