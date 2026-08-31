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
#include "rt_core.h"
#include "services.h"
#include "sysinfo.h"
#include "thread_ctx.h"
#include "time_utils.h"

int main(void) {
  /**
   * Open time logging
   */
  clock_gettime(MY_CLOCK_TYPE, &g_start_time);
  g_start_realtime = get_realtime(&g_start_time);

  /**
   * Start logging daemon
   */
  openlog("Generic Sequencer", LOG_PID | LOG_CONS, LOG_DAEMON);

  /**
   * Get system info
   */
  struct utsname info;
  if (get_sys_info(&info) == -1) {
    fprintf(stderr, "get_system_info() failure! Check syslogs at /log/var/syslogs\n");
  }
  syslog(LOG_INFO, "%s %s %s %s %s %s GNU/LINUX", COURSE_PREFIX, info.sysname, info.nodename, info.release, info.version, info.machine);

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
  thread_ctx t[NUM_THREADS];

  /**
   * Initialising the thread indices and thread creation
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    t[i].thread_idx = i + 1; // Initialising the thread indices

    ret = thread_ctx_set_attr(&t[i]); // Setting the thread object attributes
    if (ret != 0) {
      thread_ctx_destroy(&t[i]);
      closelog();
      return ret;
    }

    ret = thread_ctx_create(&t[i]); // Creating the thread objects
    if (ret != 0) {
      thread_ctx_destroy(&t[i]);
      closelog();
      return ret;
    }
  }

  /**
   * Joining threads
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    ret = thread_ctx_join(&t[i]);
    if (ret != 0) {
      thread_ctx_destroy(&t[i]);
      closelog();
      return ret;
    }
  }

  return 0;
}
