#include <time.h>

#include "app_config.h"
#include "rt_core.h"

struct timespec g_start_time;
double g_start_realtime; // Variable to store the time after conversion from timespec object

sem_t g_sem[NUM_THREADS];
volatile int g_abort_services[NUM_THREADS];

int g_timer_fd = -1;

void program_start_time(void) {
  clock_gettime(MY_CLOCK_TYPE, &g_start_time);
  g_start_realtime = get_realtime(&g_start_time);
}

double get_realtime(struct timespec *p_time) {
  double nano_to_s = (1000 * 1000 * 1000);
  double time_s = (double)p_time->tv_sec;
  double time_ns = (double)p_time->tv_nsec;
  double realtime = time_s + (time_ns / nano_to_s);
  return realtime;
}
