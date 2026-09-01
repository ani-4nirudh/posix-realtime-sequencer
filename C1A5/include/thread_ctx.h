/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file thread_ctx.h
 * @brief Header file for thread_ctx.c
 *
 * @description This file contains the thread object definition that is to be used by each service.
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

#ifndef THREAD_CTX_H
#define THREAD_CTX_H

#include <pthread.h>

typedef struct {
  // Flag variables
  int thread_idx;
  int attr_initialised;
  int thread_created;

  // Thread
  pthread_t thread;
  pthread_attr_t attr;
} thread_ctx;

/**
 * @brief Initialising the thread_idx variables
 * 
 * @param t pointer to the thread_ctx object
 * @param index intger value passed as an argument
 * 
 * @detailed This function initialises the thread_idx variable
 * */
int thread_ctx_init(thread_ctx *t, int index);

/**
 * @brief Setting the thread attributes for realtime scheduling
 *
 * @param t pointer to the thread_ctx object
 * */
int thread_ctx_set_attr(thread_ctx *t);

/**
 * @brief Function to join the threads
 * 
 * @param t pointer to the thread_ctx object
 * */
int thread_ctx_join(thread_ctx *t);

/**
 * @brief Function to destroy the thread attributes in case of cleanup or failure
 *
 * @param t pointer to the thread_ctx object
 * */
int thread_ctx_destroy(thread_ctx *t);

/**
 * @brief Function to create the thread with the custome scheduling attributes
 *
 * @param t pointer to the thread_ctx object
 * */
int thread_ctx_create(thread_ctx *t);

#endif // !THREAD_CTX_H
