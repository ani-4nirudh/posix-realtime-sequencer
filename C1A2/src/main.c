#include "sysinfo.h"

#define NUM_THREADS 128

/**
 * Define the thread parameters
 */
typedef struct {
  int thread_idx;
} thread_params_t;

/**
 * Function to calculate sum
 */
void * sum_thread( void *thread_ptr ) {
  int sum = 0;
  thread_params_t *thread_params = (thread_params_t *)(thread_ptr);
  int idx = thread_params->thread_idx;

  for ( int i = 1; i <= idx; i++ ) {
    sum = sum + i;
  }

  syslog(LOG_INFO,
         "%s: Thread idx=%d, sum[1...%d]=%d",
         COURSE_PREFIX,
         (thread_params -> thread_idx),
         (thread_params -> thread_idx),
         sum);

  return NULL;
}

int main( int argc, char *argv[] )
{
  openlog("incdecthread", LOG_PID | LOG_CONS, LOG_DAEMON);

  /**
   * Get system info
   */
  struct utsname info;
  if ( get_sys_info(&info) == -1 ) {
    printf("%s get_sys_info() failure!\n", COURSE_PREFIX);
    closelog();
    return -1;
  }

  /**
   * System info in syslog
   */
  syslog(LOG_INFO,
         "%s %s %s %s %s %s GNU/LINUX",
         COURSE_PREFIX,
         info.sysname,
         info.nodename,
         info.release,
         info.version,
         info.machine);

  pthread_t thread[NUM_THREADS];                                                                // Create the array of thread objects
  thread_params_t thread_params[NUM_THREADS];                                                   // Array of thread parameters

  for ( int i = 1; i <= NUM_THREADS; i++) {                                                     // Initialise thread indices
    thread_params[i].thread_idx = i;
  }

  for ( int i = 1; i <= NUM_THREADS; i++ ) {                                                    // Execute the sum function on different threads
    if ( pthread_create(&thread[i], NULL, sum_thread, (void *) &(thread_params[i])) != 0 ) {
      syslog(LOG_ERR, "%s pthread_create() error", COURSE_PREFIX);
      closelog();
      return -1;
    }
  }

  for ( int i = 1; i <= NUM_THREADS; i++ ) {                                                    // Join the threads
    if ( pthread_join(thread[i], NULL) != 0 ) {
      syslog(LOG_ERR, "%s pthread_join() error", COURSE_PREFIX);
      closelog();
      return -1;
    }
  }

  closelog();                                                                                   // Close logging and exit
  return 0;
}
