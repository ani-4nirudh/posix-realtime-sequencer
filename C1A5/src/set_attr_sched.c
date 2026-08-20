/**
 * @file set_thread_attr_sched.c
 * @brief Function description for set_thread_attr_sched()
 *
 * @author Anirudh Singh
 * @date 11th August 2026
 *
 */

#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "app_config.h"
#include "set_attr_sched.h"

int print_scheduler(void) {
  int sched_type = sched_getscheduler(getpid());
  if (sched_type == -1) {
    fprintf(stderr, "sched_getscheduler() failure: %s", strerror(errno));
  }

  switch (sched_type) {
  case SCHED_FIFO:
    printf("Pthread policy is SCHED_FIFO\n");
    break;
  case SCHED_OTHER:
    printf("Pthread policy is SCHED_OTHER\n");
    break;
  case SCHED_RR:
    printf("Pthread policy is SCHED_RR\n");
    break;
  default:
    printf("Pthread policy is UNKNOWN\n");
  }

  return sched_type;
}

int set_thread_attr_sched(pthread_attr_t *attr_t, int new_prio) {
  struct sched_param param;
  int ret;

  /**
   * Step 1: Initialise the threads with default attributes
   */
  ret = pthread_attr_init(attr_t); // Initialise with default attributes
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_init error: %s\n", strerror(ret));
    return ret;
  }

  /**
   * Step 2: Set scheduling policy
   */
  ret = pthread_attr_setschedpolicy(attr_t, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setschedpolicy() error: %s", strerror(ret));
    return ret;
  }

  /**
   * Step 3: Set inheritance of attributes by upcoming threads
   */
  ret = pthread_attr_setinheritsched(attr_t, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setinheritsched() error: %s\n",
            strerror(ret));
    return ret;
  }

  int min_prio = sched_get_priority_min(SCHED_FIFO);
  int max_prio = sched_get_priority_max(SCHED_FIFO);

  if ((new_prio < min_prio) || (new_prio > max_prio)) {
    fprintf(stderr,
            "Invalid range for SCHED_FIFO policy! Please enter a value between "
            "%d and %d\n",
            min_prio, max_prio);
    return -1;
  }

  memset(&param, 0, sizeof(param));
  param.sched_priority = new_prio; // Set the priority

  /**
   * Step 4: Set the attributes and parameter
   */
  ret = pthread_attr_setschedparam(attr_t, &param);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setschedparam() error: %s\n", strerror(ret));
    return ret;
  }

  /**
   * Core pinning setup
   */
  cpu_set_t cpuset;  // Data structure representing a set of CPUs
  CPU_ZERO(&cpuset); // Clearing the variable, so that it contains no CPUs
  CPU_SET(CORE_NUM, &cpuset); // Setting the core to be used by the threads

  ret = pthread_attr_setaffinity_np(attr_t, sizeof(cpuset), &cpuset);

  if (ret != 0) {
    fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}
