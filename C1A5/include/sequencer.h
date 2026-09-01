/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file sequencer.h
 * @brief Header file for sequencer.c
 *
 * @description
 * The Sequencer object is created which will be running on its own thread and thread attributes variable for real time behavior. Additionally, there are flag variables to track Sequencer thread creation and cleanup.
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

/**
 * Semaphore lifetime
 */
int sequencer_sems_init(void);
int sequencer_sems_destroy(void);

/**
 * @brief Initialise the attributes with realtime scheduling
 *
 * @param p_time Pointer to the Sequencer object
 */
int sequencer_init(Sequencer *p_seq);

/**
 * @brief Destroying the Sequencer thread object attributes 
 *
 * @param p_time Pointer to the Sequencer object
 */
int sequencer_attr_destroy(Sequencer *p_seq);

/**
 * @brief Starting the sequencer thread using the sequencer_thread_func()
 *
 * @param p_time Pointer to the Sequencer object
 */
int sequencer_start(Sequencer *p_seq);

/**
 * @brief Joining the sequencer thread and closing the timer file descriptor
 *
 * @param p_time Pointer to the Sequencer object
 */
int sequencer_join(Sequencer *p_seq);

#endif /* !SEQUENCER_H
#define */
