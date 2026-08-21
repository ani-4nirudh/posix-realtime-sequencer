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
 * @brief Main function
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>
#include <syslog.h>

#include "app_config.h"
#include "get_default_attr.h"
#include "set_attr_sched.h"
#include "sysinfo.h"

void *generic_service_1(void *thread_params_ptr) {
  int sum = 0;
  thread_params_t *ptr = (thread_params_t *)thread_params_ptr;
  int idx = ptr->thread_idx;

  int core = sched_getcpu();

  for (int i = 0; i <= idx; i++) {
    sum += i;
  }

  syslog(LOG_INFO, "%s: Thread idx=%d, sum[1...%d]=%d Running on core : %d",
         COURSE_PREFIX, idx, idx, sum, core);

  return NULL;
}

int main(void) {
  /**
   * Start logging daemon
   */
  openlog("Generic Sequencer", LOG_PID | LOG_CONS, LOG_DAEMON);

  /**
   * Get system info
   */
  struct utsname info;
  if (get_sys_info(&info) == -1) {
    fprintf(stderr,
            "get_system_info() failure! Check syslogs at /log/var/syslogs\n");
  }
  syslog(LOG_INFO, "%s %s %s %s %s %s GNU/LINUX", COURSE_PREFIX, info.sysname,
         info.nodename, info.release, info.version, info.machine);

  int ret;

  /**
   * Get default thread attributes
   */
  ret = get_default_attr();
  if (ret != 0) {
    fprintf(stderr, "get_default_attr() failure: %s\n", strerror(ret));
    return ret;
  }

  /**
   * Thread and associated variables
   */
  pthread_t threads[NUM_THREADS];
  thread_params_t thread_params[NUM_THREADS];
  pthread_attr_t attrs[NUM_THREADS];

  /**
   * Initialising the thread param variables
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    thread_params[i].thread_idx = (i + 1);
    ret = set_attr_sched(&attrs[i], &thread_params[i]);
    if (ret != 0) {
      fprintf(stderr, "set_attr_sched() failure: %s\n", strerror(ret));
      int s = pthread_attr_destroy(&attrs[i]);
      if (s != 0) {
        fprintf(stderr, "pthread_attr_destroy() failure: %s", strerror(s));
      }
      closelog();
      return ret;
    }
  }

  /**
   * Creating threads
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    ret = pthread_create(&threads[i], &attrs[i], generic_service_1,
                         (void *)&thread_params[i]);
    if (ret != 0) {
      fprintf(stderr, "pthread_create() failure: %s\n", strerror(ret));
      int s = pthread_attr_destroy(&attrs[i]);
      if (s != 0) {
        fprintf(stderr, "pthread_attr_destroy() failure: %s", strerror(s));
      }
      closelog();
      return ret;
    }
  }

  /**
   * Joining threads
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    ret = pthread_join(threads[i], NULL);
    if (ret != 0) {
      fprintf(stderr, "pthread_join() failure: %s\n", strerror(ret));
      int s = pthread_attr_destroy(&attrs[i]);
      if (s != 0) {
        fprintf(stderr, "pthread_attr_destroy() failure: %s", strerror(s));
      }
      closelog();
      return ret;
    }
  }

  return 0;
}
