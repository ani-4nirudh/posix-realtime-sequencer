#ifndef THREAD_CTX_H
#define THREAD_CTX_H

#include <pthread.h>

typedef struct {
  int thread_idx;
  int attr_initialised;
  int thread_created;

  pthread_t thread;
  pthread_attr_t attr;
} thread_ctx;

int thread_ctx_init(thread_ctx *t, int index);
int thread_ctx_set_attr(thread_ctx *t);
int thread_ctx_join(thread_ctx *t);
int thread_ctx_destroy(thread_ctx *t);
int thread_ctx_create(thread_ctx *t);

#endif // !THREAD_CTX_H
