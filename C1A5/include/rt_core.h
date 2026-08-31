#ifndef __RT_CORE_H__
#define __RT_CORE_H__

#include <semaphore.h>
#include <time.h>

#include "app_config.h"

/**
 * Creating semaphores
 */
extern sem_t g_sem[NUM_THREADS];

/**
 * Creating volatile flags for the services
 */
extern volatile int g_abort_services[NUM_THREADS];

/**
 * Start time for the program 
 */
extern struct timespec g_start_time;
extern double g_start_realtime; // Variable to store the time after conversion from timespec object

extern int g_timer_fd;
extern unsigned long long int g_seq_cnt;
extern unsigned long long int g_sequence_periods;

#endif /* ! __RT_CORE_H__
#define __RT_CORE_H__ */
