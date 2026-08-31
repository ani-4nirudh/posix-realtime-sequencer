#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "sequencer.h"

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;

} Sequencer;

/**
 * Setting the sequencer thread attributes and priority
 */
int sequencer_attr_sched(Sequencer *seq) {

  /**
   * Initialise attributes for the sequencer thread
   */
  int ret = pthread_attr_init(p_attr);
  if (ret != 0) {
    fprintf(stderr, "sequencer_attr_sched: pthread_attr_init() failure %s\n", strerror(ret));
    return ret;
  }

  /**
    * Set schduling policy for the sequencer thread
  */
  ret = pthread_attr_setschedpolicy(p_attr, SCHED_FIFO);
  if (ret != 0) {
    fprintf(stderr, "sequencer_attr_sched: pthread_attr_setschedpolicy() failure %s\n", strerror(ret));
    return ret;
  }

  /**
   * Set inheritance of the thread attributes
   */
  ret = pthread_attr_setinheritsched(p_attr, PTHREAD_EXPLICIT_SCHED);
  if (ret != 0) {
    fprintf(stderr, "sequencer_attr_sched: pthread_attr_setinheritsched() failure %s\n", strerror(ret));
    return ret;
  }

  /**
   * Setup scheduling priority
   */
  struct sched_param param;
  memset(&param, 0, sizeof(param));
  param.sched_priority = sched_get_priority_max(SCHED_FIFO);
  ret = pthread_attr_setschedparam(p_attr, &param);

  /**
   * Setup core pinning to core 1
   */
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(SEQUENCER_CORE, &set);
  ret = pthread_attr_setaffinity_np(p_attr, sizeof(set), &set);
  if (ret != 0) {
    fprintf(stderr, "sequencer_attr_sched: pthread_attr_setaffinity_np() failure %s\n", strerror(ret));
    return ret;
  }

  /**
   * Return 0 on success
   */
  return 0;
}

int sequencer_init(pthread_attr_t *p_attr) {
  pthread_t seq_thread;

  return 0;
}

pthread_attr_t seq_attr; // Creating the sequencer attribute

void *sequencer_thread_func(void *) {
  if (sequencer_attr_sched(&seq_attr) != 0) {
    pthread_attr_destroy(&sequencer_attr);
  }
}
