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
