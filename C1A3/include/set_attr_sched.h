/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file set_attr_sched.h
 * @brief Header file for src/set_attr_sched.c
 *
 * @author Anirudh Singh
 * @date 11th August 2026
 *
 */

#ifndef __SET_ATTR_SCHED_H__
#define __SET_ATTR_SCHED_H__

#include <pthread.h>

/**
 * @brief Set the thread attributes and scheduling policy using the following
 * function
 *
 * @param *attr_t Pointer to the thread attribute
 * @param new_prio Setting the priority number for the scheduling policy
 *
 * @detailed The function will use the mentioned parameters to set a custom
 * scheduling policy to run its threads
 * 1. Initialise the threads with default attributes
 * 2. Set the scheduling policy
 * 3. Making sure that threads inherit this sched policy
 * 4. Setting the priority number for the threads with this policy
 * */

int set_thread_attr_sched(pthread_attr_t *attr_t, int new_prio);

#endif /* !__SET_ATTR_SCHED_H__                                                \
#define __SET_ATTR_SCHED_H__ */
