/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file app_config.h
 * @brief
 *
 * @description
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

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
