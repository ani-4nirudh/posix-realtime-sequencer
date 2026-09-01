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

#ifndef RT_CORE_H
#define RT_CORE_H

#include <semaphore.h>
#include <time.h>

#include "app_config.h"
#include "rt_core.h"

/**
 * Start time for the program 
 */
extern struct timespec g_start_time;
extern double g_start_realtime; // Variable to store the time after conversion from timespec object

extern int g_timer_fd;

extern sem_t g_sem[NUM_THREADS];
extern volatile int g_abort_services[NUM_THREADS];

void program_start_time(void);
double get_realtime(struct timespec *p_time);

#endif /* ! RT_CORE_H
#define RT_CORE_H */
