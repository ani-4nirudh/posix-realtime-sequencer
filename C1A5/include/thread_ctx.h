#ifndef __THREAD_CTX_H__
#define __THREAD_CTX_H__

#include <pthread.h>

typedef struct {
  int thread_idx;
  pthread_t thread;
  pthread_attr_t attr;
} thread_ctx;

int thread_ctx_set_attr(thread_ctx *t);
int thread_ctx_join(thread_ctx *t);
int thread_ctx_destroy(thread_ctx *t);
int thread_ctx_create(thread_ctx *t);

#endif // !__THREAD_CTX_H__
