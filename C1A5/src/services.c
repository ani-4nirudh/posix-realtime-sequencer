#include <sched.h>
#include <stdio.h>
#include <syslog.h>
#include <time.h>

#include "rt_core.h"
#include "services.h"
#include "thread_ctx.h"
#include "time_utils.h"

void *generic_service(void *thread_ctx_ptr) {
  int sum = 0;
  thread_ctx *ptr = (thread_ctx *)thread_ctx_ptr;
  int idx = ptr->thread_idx;         // idx was increased by 1 when creating the threads
  int sem_idx = idx - 1;             // sem[0] for thread[0]
  unsigned long long exec_count = 0; // For saving frequency of execution of a service

  // For calculating time
  struct timespec current_time_val;
  double current_realtime;

  // Start up processing and resource initialization
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = get_realtime(&current_time_val);
  syslog(LOG_CRIT, "%s: Service %d thread @ sec=%6.9lf\n", COURSE_PREFIX, idx, (current_realtime - g_start_realtime));
  printf("S%d thread @ sec=%6.9lf\n", idx, (current_realtime - g_start_realtime));

  // Wait for the semaphore
  while (!g_abort_services[sem_idx]) {
    printf("Waiting on the sempahore g_sem[%d] for thread[%d]\n", sem_idx, sem_idx);
    sem_wait(&g_sem[sem_idx]);

    // Check if abort on the thread has been called
    if (g_abort_services[sem_idx]) {
      break;
    }

    int core = sched_getcpu(); // Get the core number thread is running on

    // The work
    for (int i = 0; i <= idx; i++) {
      sum += i;
    }

    // Increase the counter
    exec_count++;

    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = get_realtime(&current_time_val);
    syslog(LOG_CRIT,
           "%s: Service %d running on Thread %d Core : %d, for release %llu sum[1...%d]=%d",
           COURSE_PREFIX,
           idx,
           sem_idx,
           core,
           exec_count,
           idx,
           sum);
  }

  syslog(LOG_INFO, "%s: Service %d exiting after %llu release(s) ...", COURSE_PREFIX, idx, exec_count);
  return NULL;
}
