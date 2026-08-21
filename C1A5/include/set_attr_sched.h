#ifndef __SET_ATTR_SCHED_H__
#define __SET_ATTR_SCHED_H__

#include <pthread.h>

typedef struct {
  int thread_idx;
} thread_params_t;

int set_attr_sched(pthread_attr_t *p_attr, thread_params_t *thread_params_ptr);

#endif // !_SET_ATTR_SCHED_H__
