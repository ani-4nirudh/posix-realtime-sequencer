
#include <pthread.h>
#include <stdio.h>
#include <string.h>

int get_default_attr(void) {
  pthread_attr_t default_attr;

  int ret, i;

  /**
   * Initialise the attribute
   */
  ret = pthread_attr_init(&default_attr);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_init() failure: %s", strerror(ret));
    return ret;
  }

  /**
   * Get initial scheduling policy
   */
  ret = pthread_attr_getschedpolicy(&default_attr, &i);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_getschedpolicy() failure: %s", strerror(ret));
    return ret;
  }
  switch (i) {
  case SCHED_FIFO:
    fprintf(stdout, "INITIAL Scheduling Policy: SCHED_FIFO\n");
    break;
  case SCHED_RR:
    fprintf(stdout, "INITIAL Scheduling Policy: SCHED_RR\n");
    break;
  case SCHED_OTHER:
    fprintf(stdout, "INITIAL Scheduling Policy: SCHED_OTHER\n");
    break;
  default:
    fprintf(stdout, "INITIAL Scheduling Policy: UNKNOWN\n");
    break;
  }

  /**
   * Get inheritance schdeuling status
   */
  ret = pthread_attr_getinheritsched(&default_attr, &i);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_getinheritsched() failure: %s", strerror(ret));
    return ret;
  }
  switch (i) {
  case PTHREAD_EXPLICIT_SCHED:
    fprintf(stdout, "INITIAL Scheduling Inheritance: PTHREAD_EXPLICIT_SCHED\n");
    break;
  case PTHREAD_INHERIT_SCHED:
    fprintf(stdout, "INITIAL Scheduling Inheritance: PTHREAD_INHERIT_SCHED\n");
    break;
  default:
    fprintf(stdout, "INITIAL Scheduling Inheritance: UNKNOWN\n");
    break;
  }

  /**
   * Destroy the attributes(
   */
  ret = pthread_attr_destroy(&default_attr);
  if (ret != 0) {
    fprintf(stderr, "pthread_attr_getinheritsched() failure: %s", strerror(ret));
    return ret;
  }

  return 0;
}
