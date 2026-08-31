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
#include "sequencer.h"
#include "services.h"
#include "sysinfo.h"
#include "thread_ctx.h"

int main(void) {
  int ret = 0;
  int cleanup_ret = 0;

  /**
   * Open time logging
   */
  program_start_time();

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

  /**
   * Get default thread attributes
   */
  ret = get_default_attr();
  if (ret != 0) {
    fprintf(stderr, "get_default_attr() failure: %s\n", strerror(ret));
    return ret;
  }

  Sequencer seq;
  ret = sequencer_sems_init();
  if (ret != 0) {
    sequencer_sems_destroy();
    return ret;
  }

  ret = sequencer_init(&seq);
  if (ret != 0) {
    if (seq.attr_initialised) {
      sequencer_attr_destroy(&seq);
    }
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
    thread_ctx_init(&t[i], i); // Initialising the thread indices

    ret = thread_ctx_set_attr(&t[i]); // Setting the thread object attributes
    if (ret != 0) {
      closelog();
      goto cleanup;
    }

    ret = thread_ctx_create(&t[i]); // Creating the thread objects
    if (ret != 0) {
      closelog();
      goto cleanup;
    }
  }

  sequencer_start(&seq);

  /**
   * Joining threads
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    ret = thread_ctx_join(&t[i]);
    if (ret != 0) {
      closelog();
      goto cleanup;
    }
  }

  sequencer_join(&seq);

  /**
   * Return 0 on success
   */
  return 0;

cleanup:
  for (int i = 0; i < NUM_THREADS; i++) {
    if (t[i].thread_created) {
      cleanup_ret = thread_ctx_join(&t[i]);
      if (cleanup_ret != 0) {
        fprintf(stderr, "thread_ctx_create() failure: %s\n", strerror(cleanup_ret));
      }
    }
    if (t[i].attr_initialised) {
      cleanup_ret = thread_ctx_destroy(&t[i]);
      if (cleanup_ret != 0) {
        fprintf(stderr, "thread_ctx_destroy() failure: %s\n", strerror(cleanup_ret));
      }
    }
  }
  return ret;
}
