/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file thread_ctx.c
 *
 * @brief Source file for thread_ctx.h
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "services.h"
#include "thread_ctx.h"

int thread_ctx_init(thread_ctx *t, int index) {
  memset(t, 0, sizeof(*t));
  t->thread_idx = index;

  return 0;
}

int thread_ctx_set_attr(thread_ctx *t) {
  int ret = 0;
  int fail_ret = 0;

  /**
   * Intialise attributes
   */
  ret = pthread_attr_init(&t->attr);
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_set_attr: pthread_attr_init() failure: %s", strerror(ret));
    return ret;
  }

  t->attr_initialised = 1;

  /**
   * Set scheduling policy
   */
  ret = pthread_attr_setschedpolicy(&t->attr, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_set_attr: pthread_attr_setschedpolicy() failure: %s", strerror(ret));
    goto fail;
  }

  /**
   * Set attribute inheritance by subsequent threads
   */
  ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_set_attr: pthread_attr_setinheritsched() failure: %s", strerror(ret));
    goto fail;
  }

  /**
   * Setting up the scheduling parameter for priority
   */
  int max_prio = sched_get_priority_max(SCHED_FIFO);
  if (max_prio == -1) {
    fprintf(stderr, "sched_get_priority_max() error: %s\n", strerror(errno));
    goto fail;
  }

  /**
   * Higher the index value, lower the priority compared to the other threads
   * Reserving prio 99 for the sequencer thread
   * Thread[0] = 98
   * Thread[1] = 97
   */
  int idx = (t->thread_idx);     // Gives 0 for the first thread
  int prio = max_prio - idx - 1; // Prio 98 for first thread
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = prio;

  ret = pthread_attr_setschedparam(&t->attr, &param);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setschedparam() error: %s\n", strerror(ret));
    goto fail;
  }

  /**
   * Core pinning logic:
   * Even thread indices run on core 2
   * Odd thread indices run on core 3
   */
  cpu_set_t set;
  CPU_ZERO(&set);
  if ((idx % 2) == 0) {
    CPU_SET(EVEN_THREAD_CORE, &set);
  } else {
    CPU_SET(ODD_THREAD_CORE, &set);
  }
  ret = pthread_attr_setaffinity_np(&t->attr, sizeof(set), &set);
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_set_attr: pthread_attr_setaffinity_np() failure: %s", strerror(ret));
    goto fail;
  }

  /**
   * Return 0 on success
   */
  return 0;

fail:
  fail_ret = pthread_attr_destroy(&t->attr);
  t->attr_initialised = 0;
  if (fail_ret != 0) {
    fprintf(stderr, "thread_ctx_set_attr() -> pthread_attr_destroy() failure: %s", strerror(fail_ret));
  }
  return ret;
}

int thread_ctx_create(thread_ctx *t) {
  if (!t->attr_initialised) {
    return EINVAL;
  }

  int ret = pthread_create(&t->thread, &t->attr, generic_service, (void *)t);
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_create() -> pthread_create() failure: %s\n", strerror(ret));
    return ret;
  }
  t->thread_created = 1;

  ret = pthread_attr_destroy(&t->attr);
  t->attr_initialised = 0;
  if (ret != 0) {
    fprintf(stderr, "thread_ctx_create() -> pthread_attr_destroy() failure: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}

int thread_ctx_destroy(thread_ctx *t) {
  if ((t == NULL) || (!t->attr_initialised)) {
    return EINVAL;
  }

  int ret = pthread_attr_destroy(&t->attr);
  t->attr_initialised = 0;
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_destroy() failure: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}

int thread_ctx_join(thread_ctx *t) {
  if ((t == NULL) || (!t->thread_created)) {
    fprintf(stderr, "thread_ctx: pthread_attr_join() failure!\n");
    return EINVAL;
  }

  int ret = pthread_join(t->thread, NULL);
  if (ret != 0) {
    fprintf(stderr, "pthread_join() failure: %s\n", strerror(ret));
    return ret;
  }

  t->thread_created = 0;
  return 0;
}
