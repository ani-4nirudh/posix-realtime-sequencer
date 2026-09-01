/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file services.c
 * @brief Functions to implement the service used by each thread
 *
 * @description
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

#include <sched.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>

#include "rt_core.h"
#include "services.h"
#include "thread_ctx.h"

void *generic_service(void *thread_ctx_ptr) {
  thread_ctx *ptr = (thread_ctx *)thread_ctx_ptr;
  int idx = ptr->thread_idx;
  int sem_idx = idx;                 // sem[0] for thread[0]
  unsigned long long exec_count = 0; // For saving frequency of execution of a service

  // For calculating time
  struct timespec current_time_val;
  double current_realtime;

  // Wait for the semaphore
  while (!g_abort_services[sem_idx]) {
    // printf("Waiting on the sempahore g_sem[%d] for thread[%d]\n", sem_idx, sem_idx);
    sem_wait(&g_sem[sem_idx]);

    /**
    * Check for flags to abort the service
    */
    if (g_abort_services[sem_idx]) {
      break;
    }

    exec_count++; // Increase the release counter

    int core = sched_getcpu(); // Get the core number thread is running on

    /**
     * Get the time elapsed since the start of program
     */
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = get_realtime(&current_time_val);
    double elapsed = current_realtime - g_start_realtime;

    syslog(LOG_CRIT,
           "%s: Service %d running on Thread %d Core : %d, for release %llu @ sec=%6.9lf\n",
           COURSE_PREFIX,
           idx,
           sem_idx,
           core,
           exec_count,
           elapsed);
  }

  syslog(LOG_INFO, "%s: Service %d exiting after %llu release(s) ...", COURSE_PREFIX, idx, exec_count);
  return NULL;
}
