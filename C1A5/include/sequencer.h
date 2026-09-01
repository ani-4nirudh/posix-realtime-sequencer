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

#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <pthread.h>

typedef struct {
  pthread_t thread;
  pthread_attr_t attr;

  int attr_initialised;
  int thread_created;
} Sequencer;

int sequencer_sems_init(void);
int sequencer_sems_destroy(void);

int sequencer_init(Sequencer *p_seq);
int sequencer_attr_destroy(Sequencer *p_seq);
int sequencer_start(Sequencer *p_seq);
int sequencer_join(Sequencer *p_seq);

#endif /* !SEQUENCER_H
#define */
