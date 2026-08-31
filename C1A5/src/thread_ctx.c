#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "services.h"
#include "thread_ctx.h"

int thread_ctx_set_attr(thread_ctx *t) {
  int ret;

  /**
   * Intialise attributes
   */
  ret = pthread_attr_init(&t->attr);
  if (ret != 0) {
    fprintf(stderr, "set_attr_sched: pthread_attr_init() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Set scheduling policy
   */
  ret = pthread_attr_setschedpolicy(&t->attr, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "set_attr_sched: pthread_attr_setschedpolicy() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Set attribute inheritance by subsequent threads
   */
  ret = pthread_attr_setinheritsched(&t->attr, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "set_attr_sched: pthread_attr_setinheritsched() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Setting up the scheduling parameter for priority
   */
  int max_prio = sched_get_priority_max(SCHED_FIFO);
  if (max_prio == -1) {
    fprintf(stderr, "sched_get_priority_max() error: %s\n", strerror(errno));
    return max_prio;
  }

  /**
   * Higher the index value, lower the priority compared to the other threads
   * Reserving prio 99 for the sequencer thread
   * Thread[0] = 98
   * Thread[1] = 97
   */
  int idx = (t->thread_idx) - 1; // Gives 0 for the first thread
  int prio = max_prio - idx - 1; // Prio 98 for first thread
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = prio;

  ret = pthread_attr_setschedparam(&t->attr, &param);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setschedparam() error: %s\n", strerror(ret));
    return ret;
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
    fprintf(stderr, "set_attr_sched: pthread_attr_setaffinity_np() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Return 0 on success
   */
  return ret;
}

int thread_ctx_create(thread_ctx *t) {
  int ret;
  ret = pthread_create(&t->thread, &t->attr, generic_service, (void *)t);
  if (ret != 0) {
    fprintf(stderr, "pthread_create() failure: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}

int thread_ctx_destroy(thread_ctx *t) {
  int ret = pthread_attr_destroy(&t->attr);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_destroy() failure: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}

int thread_ctx_join(thread_ctx *t) {
  int ret = pthread_join(t->thread, NULL);
  if (ret != 0) {
    fprintf(stderr, "pthread_join() failure: %s\n", strerror(ret));
    return ret;
  }

  return 0;
}
