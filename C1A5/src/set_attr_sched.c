#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "app_config.h"
#include "set_attr_sched.h"

int set_attr_sched(pthread_attr_t *p_attr, thread_params_t *thread_params_ptr) {
  int ret;

  /**
   * Intialise attributes
   */
  ret = pthread_attr_init(p_attr);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_init() failure: %s", strerror(ret));
  }

  /**
   * Set scheduling policy
   */
  ret = pthread_attr_setschedpolicy(p_attr, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setschedpolicy() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Set attribute inheritance by subsequent threads
   */
  ret = pthread_attr_setinheritsched(p_attr, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setinheritsched() failure: %s",
            strerror(ret));
    return ret;
  }

  /**
   * Setting up the scheduling parameter for priority
   */
  int max_prio = sched_get_priority_max(SCHED_FIFO);

  /**
   * Higher the index value, lower the priority compared to the other threads
   * Thread[0] = 99
   * Thread[1] = 98
   */
  int idx = (thread_params_ptr->thread_idx) - 1;
  int prio = max_prio - idx;
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = prio;

  ret = pthread_attr_setschedparam(p_attr, &param);

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
  ret = pthread_attr_setaffinity_np(p_attr, sizeof(set), &set);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_setaffinity_np() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Return 0 on success
   */
  return ret;
}
