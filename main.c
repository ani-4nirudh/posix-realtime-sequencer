#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <sys/utsname.h>

static const char COURSE_PREFIX[] = "[COURSE:1][ASSIGNMENT:1]";

/**
 * Get system info
 */
int get_system_info(struct utsname *ptr) {
  if ( uname(ptr) == -1 ) {
    syslog(LOG_ERR,
           "%s uname() failure: %s",
           COURSE_PREFIX,
           strerror(errno));
    return -1;
  }

  return 0;
}

/**
 * Function to pass to pthread_create
 */
void * thread_msg(void *arg) {
  syslog(LOG_INFO, "%s Hello World from Thread!", COURSE_PREFIX);
  return NULL;
}

int main(int argc, char *argv[])
{
  openlog("pthread", LOG_PID | LOG_CONS, LOG_DAEMON);                 // Open logging

  /**
   * Get System Info
   */
  struct utsname info;
  if ( get_system_info(&info) == -1 ) {
    printf("get_system_info() failure! Check syslogs!\n");
    closelog();
    return -1;
  }

  syslog(LOG_INFO,                                                    // Printing uname output to syslog
         "%s %s %s %s %s %s GNU/LINUX",
         COURSE_PREFIX,
         info.sysname,
         info.nodename,
         info.release,
         info.version,
         info.machine);

  syslog(LOG_INFO, "%s Hello World from Main!", COURSE_PREFIX);

  /**
    * Creating thread
    */
  pthread_t thread_1;
  if ( pthread_create(&thread_1, NULL, thread_msg, NULL) != 0 ) {
    syslog(LOG_ERR, "%s pthread_create() failure", COURSE_PREFIX);
    closelog();
    return -1;
  }

  if ( pthread_join(thread_1, NULL) != 0 ) {
    syslog(LOG_ERR, "%s pthread_join() failure", COURSE_PREFIX);
    closelog();
    return -1;
  }

  closelog();
  return 0;
}
