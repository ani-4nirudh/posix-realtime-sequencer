#include <errno.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/timerfd.h>
#include <syslog.h>
#include <unistd.h>

#include "app_config.h"
#include "rt_core.h"
#include "sequencer.h"

/**
 * Initialising semaphores
 */
int sequencer_sems_init(void) {
  for (int i = 0; i < NUM_THREADS; i++) {
    g_abort_services[i] = 0;
    int ret = sem_init(&g_sem[i], 0, 0);
    if (ret == -1) {
      fprintf(stderr, "sequencer_sem_init: sem_init() failure: %s\n", strerror(errno));
      return ret;
    }
  }

  return 0;
}

/**
 * Destroying semaphores
 */
int sequencer_sems_destroy(void) {
  for (int i = 0; i < NUM_THREADS; i++) {
    int ret = sem_destroy(&g_sem[i]);
    if (ret == -1) {
      fprintf(stderr, "sequencer_sem_destroy: sem_destroy() failure: %s\n", strerror(errno));
      return ret;
    }
  }

  return 0;
}

/**
 * Setting the sequencer thread attributes and priority
 */
int sequencer_init(Sequencer *p_seq) {

  /**
   * Initialise the Sequencer object to 0
   */
  memset(p_seq, 0, sizeof(*p_seq));

  /** 
   * Return status
   */
  int ret = 0;
  int fail_ret = 0;

  /**
   * Initialise attributes for the sequencer thread
   */
  ret = pthread_attr_init(&p_seq->attr);
  if (ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_init() failure %s\n", strerror(ret));
    return ret;
  }

  p_seq->attr_initialised = 1; // Thread attributes have been successfully initialised

  /**
    * Set schduling policy for the sequencer thread
  */
  ret = pthread_attr_setschedpolicy(&p_seq->attr, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_setschedpolicy() failure %s\n", strerror(ret));
    goto fail;
  }

  /**
   * Set inheritance of the thread attributes
   */
  ret = pthread_attr_setinheritsched(&p_seq->attr, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_setinheritsched() failure %s\n", strerror(ret));
    goto fail;
  }

  /**
   * Setup scheduling priority
   */
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = sched_get_priority_max(SCHED_FIFO);
  if (param.sched_priority == -1) {
    fprintf(stderr, "sequencer_init: sched_get_priority_max() failure %s\n", strerror(errno));
    goto fail;
  }
  ret = pthread_attr_setschedparam(&p_seq->attr, &param);
  if (ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_setschedparam() failure %s\n", strerror(ret));
    goto fail;
  }

  /**
   * Setup core pinning to core 1
   */
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(SEQUENCER_CORE, &set);
  ret = pthread_attr_setaffinity_np(&p_seq->attr, sizeof(set), &set);
  if (ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_setaffinity_np() failure %s\n", strerror(ret));
    goto fail;
  }

  /**
   * Return 0 on success
   */
  return ret;

fail:
  fail_ret = pthread_attr_destroy(&p_seq->attr);
  p_seq->attr_initialised = 0;
  if (fail_ret != 0) {
    fprintf(stderr, "sequencer_init: pthread_attr_destroy() failure %s\n", strerror(fail_ret));
  }

  return ret;
}

int sequencer_attr_destroy(Sequencer *p_seq) {
  if (!p_seq->attr_initialised) {
    return EINVAL;
  }

  int ret = pthread_attr_destroy(&p_seq->attr);
  p_seq->attr_initialised = 0;
  if (ret != 0) {
    fprintf(stderr, "sequencer_attr_destroy: pthread_attr_destroy() failure %s\n", strerror(ret));
  }

  return ret;
}

int sequencer_join(Sequencer *p_seq) {
  if (!p_seq->thread_created) {
    fprintf(stderr, "sequencer_join: No thread has been created for the given Sequencer object!\n");
    return EINVAL;
  }

  int ret = pthread_join(p_seq->thread, NULL);
  if (ret != 0) {
    fprintf(stderr, "sequencer_join: pthread_join() failure %s\n", strerror(ret));
    return ret;
  }

  p_seq->thread_created = 0;
  return 0;
}

void *sequencer_thread_func(void *ptr) {
  Sequencer *ptr_sequencer = (Sequencer *)(ptr);

  syslog(LOG_INFO, "Sequencer thread started for object %p\n", (void *)ptr_sequencer);

  struct itimerspec its;
  its.it_value.tv_sec = BASE_PERIOD_NS / 1000000000ULL;
  its.it_value.tv_nsec = BASE_PERIOD_NS % 1000000000ULL;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = BASE_PERIOD_NS;

  g_timer_fd = timerfd_create(TIMERFD_CLOCK_TYPE, 0);
  if (g_timer_fd == -1) {
    fprintf(stderr, "sequencer_thread_func: timerfd_create() failure %s\n", strerror(errno));
    return NULL;
  }

  int ret = timerfd_settime(g_timer_fd, 0, &its, NULL);
  if (ret == -1) {
    fprintf(stderr, "sequencer_thread_func: timerfd_create() failure %s\n", strerror(errno));
    close(g_timer_fd);
    g_timer_fd = -1;
    return NULL;
  }

  unsigned long long tick_count = 0;

  while (tick_count < TOTAL_TICKS) {
    uint64_t expirations;

    ssize_t n = read(g_timer_fd, &expirations, sizeof(expirations));
    if (n == -1) {
      fprintf(stderr, "sequencer_thread_func: read() failure %s\n", strerror(errno));
      return NULL;
    }

    for (uint64_t e = 0; e < expirations; e++) {
      tick_count++;

      for (int i = 0; i < NUM_THREADS; i++) {
        if ((tick_count % THREAD_RATES[i]) == 0) {
          ret = sem_post(&g_sem[i]);
          if (ret == -1) {
            fprintf(stderr, "sequencer_thread_func: sem_post() failure %s\n", strerror(errno));
            return NULL;
          }
        }
      }
    }
  }

  /**
   * Shutdown services
   */
  for (int i = 0; i < NUM_THREADS; i++) {
    g_abort_services[i] = 1;
    sem_post(&g_sem[i]);
  }
  syslog(LOG_CRIT,
         "Sequencer thread exiting normally: tick_count = %llu, TOTAL_TICKS = %llu\n",
         (unsigned long long)tick_count,
         (unsigned long long)TOTAL_TICKS);

  close(g_timer_fd);
  g_timer_fd = -1;
  return NULL;
}

int sequencer_start(Sequencer *p_seq) {
  int ret = pthread_create(&p_seq->thread, &p_seq->attr, sequencer_thread_func, (void *)p_seq);
  if (ret != 0) {
    fprintf(stderr, "sequencer_start: pthread_create() failure %s\n", strerror(ret));
    return ret;
  }

  p_seq->thread_created = 1;
  return 0;
}
