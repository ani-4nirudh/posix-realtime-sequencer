// Sam Siewert, December 2020 (original)
// Modified: timerfd-based sequencer (no SIGALRM / signal handler)
//
// Only the TIMING SOURCE has changed relative to the original seqgen3.c.
// All service functions, thread creation, SCHED_FIFO priorities, and
// CPU affinity assignments are unchanged in structure and intent.
//
// Key differences from the original:
//   1) No timer_create()/signal(SIGALRM,...)/timer_settime() combo.
//   2) A timerfd is created and armed instead.
//   3) The Sequencer's dispatch logic runs inside a normal blocking
//      read() loop in a dedicated real-time thread, NOT inside a
//      signal handler. This removes all async-signal-safety concerns
//      (printf/syslog become safe to call; abort flags and seqCnt no
//      longer need to be sig_atomic_t because no signal handler ever
//      touches them).
//   4) Missed periods are detected directly: read() returns a uint64_t
//      count of expirations since the last read, so an overrun is
//      visible instead of silently dropped (unlike SIGALRM coalescing).

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <syslog.h>
#include <sys/timerfd.h>   // timerfd_create / timerfd_settime

#define USEC_PER_MSEC     (1000)
#define NANOSEC_PER_MSEC  (1000000)
#define NANOSEC_PER_SEC   (1000000000)
#define NUM_CPU_CORES     (4)
#define TRUE  (1)
#define FALSE (0)

#define NUM_THREADS (7)

// timerfd_create only accepts CLOCK_MONOTONIC, CLOCK_REALTIME, or
// CLOCK_BOOTTIME -- NOT CLOCK_MONOTONIC_RAW. Use CLOCK_MONOTONIC for
// the timer itself; keep MONOTONIC_RAW (or whatever you prefer) for
// timestamping inside the services if desired.
#define MY_TIMERFD_CLOCK  CLOCK_MONOTONIC
#define MY_CLOCK_TYPE     CLOCK_MONOTONIC_RAW

int abortTest = FALSE;
int abortS1 = FALSE, abortS2 = FALSE, abortS3 = FALSE, abortS4 = FALSE,
    abortS5 = FALSE, abortS6 = FALSE, abortS7 = FALSE;
sem_t semS1, semS2, semS3, semS4, semS5, semS6, semS7;
struct timespec start_time_val;
double start_realtime;
unsigned long long sequencePeriods;

static int timer_fd = -1;               // replaces timer_t timer_1
static unsigned long long seqCnt = 0;

typedef struct {
  int threadIdx;
} threadParams_t;

void *Sequencer(void *arg);   // now a normal thread function, not a signal handler

void *Service_1(void *threadp);
void *Service_2(void *threadp);
void *Service_3(void *threadp);
void *Service_4(void *threadp);
void *Service_5(void *threadp);
void *Service_6(void *threadp);
void *Service_7(void *threadp);

double getTimeMsec(void);
double realtime(struct timespec *tsptr);
void print_scheduler(void);

int main(void) {
  struct timespec current_time_val, current_time_res;
  double current_realtime, current_realtime_res;

  int i, rc, scope;
  cpu_set_t threadcpu;
  cpu_set_t allcpuset;

  pthread_t threads[NUM_THREADS];
  pthread_t seq_thread;
  threadParams_t threadParams[NUM_THREADS];
  pthread_attr_t rt_sched_attr[NUM_THREADS];
  pthread_attr_t seq_sched_attr;
  int rt_max_prio, rt_min_prio, cpuidx;

  struct sched_param rt_param[NUM_THREADS];
  struct sched_param seq_param;
  struct sched_param main_param;

  pthread_attr_t main_attr;
  pid_t mainpid;

  printf("Starting High Rate Sequencer Demo (timerfd)\n");
  clock_gettime(MY_CLOCK_TYPE, &start_time_val);
  start_realtime = realtime(&start_time_val);
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  clock_getres(MY_CLOCK_TYPE, &current_time_res);
  current_realtime_res = realtime(&current_time_res);
  printf("START High Rate Sequencer @ sec=%6.9lf with resolution %6.9lf\n",
         (current_realtime - start_realtime), current_realtime_res);
  syslog(LOG_CRIT,
         "START High Rate Sequencer @ sec=%6.9lf with resolution %6.9lf\n",
         (current_realtime - start_realtime), current_realtime_res);

  printf("System has %d processors configured and %d available.\n",
         get_nprocs_conf(), get_nprocs());

  CPU_ZERO(&allcpuset);
  for (i = 0; i < NUM_CPU_CORES; i++)
    CPU_SET(i, &allcpuset);
  printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));

  if (sem_init(&semS1, 0, 0)) { printf("Failed to initialize S1 semaphore\n"); exit(-1); }
  if (sem_init(&semS2, 0, 0)) { printf("Failed to initialize S2 semaphore\n"); exit(-1); }
  if (sem_init(&semS3, 0, 0)) { printf("Failed to initialize S3 semaphore\n"); exit(-1); }
  if (sem_init(&semS4, 0, 0)) { printf("Failed to initialize S4 semaphore\n"); exit(-1); }
  if (sem_init(&semS5, 0, 0)) { printf("Failed to initialize S5 semaphore\n"); exit(-1); }
  if (sem_init(&semS6, 0, 0)) { printf("Failed to initialize S6 semaphore\n"); exit(-1); }
  if (sem_init(&semS7, 0, 0)) { printf("Failed to initialize S7 semaphore\n"); exit(-1); }

  mainpid = getpid();

  rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  rt_min_prio = sched_get_priority_min(SCHED_FIFO);

  rc = sched_getparam(mainpid, &main_param);
  main_param.sched_priority = rt_max_prio;
  rc = sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
  if (rc < 0) perror("main_param");
  print_scheduler();

  pthread_attr_getscope(&main_attr, &scope);
  if (scope == PTHREAD_SCOPE_SYSTEM) printf("PTHREAD SCOPE SYSTEM\n");
  else if (scope == PTHREAD_SCOPE_PROCESS) printf("PTHREAD SCOPE PROCESS\n");
  else printf("PTHREAD SCOPE UNKNOWN\n");

  printf("rt_max_prio=%d\n", rt_max_prio);
  printf("rt_min_prio=%d\n", rt_min_prio);

  // --- Service thread attributes: even idx -> core 2, odd idx -> core 3 ---
  for (i = 0; i < NUM_THREADS; i++) {
    if (i % 2 == 0) {
      CPU_ZERO(&threadcpu);
      cpuidx = 2;
      CPU_SET(cpuidx, &threadcpu);
    } else {
      CPU_ZERO(&threadcpu);
      cpuidx = 3;
      CPU_SET(cpuidx, &threadcpu);
    }
    rc = pthread_attr_init(&rt_sched_attr[i]);
    rc = pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
    rc = pthread_attr_setschedpolicy(&rt_sched_attr[i], SCHED_FIFO);
    rc = pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

    rt_param[i].sched_priority = rt_max_prio - i;
    pthread_attr_setschedparam(&rt_sched_attr[i], &rt_param[i]);
    threadParams[i].threadIdx = i;
  }
  printf("Service threads will run on cores 2/3 by even/odd index\n");

  // --- Create Service_1..Service_7 exactly as before ---
  rt_param[0].sched_priority = rt_max_prio - 1;
  pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
  rc = pthread_create(&threads[0], &rt_sched_attr[0], Service_1, (void *)&threadParams[0]);
  if (rc < 0) perror("pthread_create for service 1"); else printf("pthread_create successful for service 1\n");

  rt_param[1].sched_priority = rt_max_prio - 2;
  pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
  rc = pthread_create(&threads[1], &rt_sched_attr[1], Service_2, (void *)&threadParams[1]);
  if (rc < 0) perror("pthread_create for service 2"); else printf("pthread_create successful for service 2\n");

  rt_param[2].sched_priority = rt_max_prio - 3;
  pthread_attr_setschedparam(&rt_sched_attr[2], &rt_param[2]);
  rc = pthread_create(&threads[2], &rt_sched_attr[2], Service_3, (void *)&threadParams[2]);
  if (rc < 0) perror("pthread_create for service 3"); else printf("pthread_create successful for service 3\n");

  rt_param[3].sched_priority = rt_max_prio - 4;
  pthread_attr_setschedparam(&rt_sched_attr[3], &rt_param[3]);
  rc = pthread_create(&threads[3], &rt_sched_attr[3], Service_4, (void *)&threadParams[3]);
  if (rc < 0) perror("pthread_create for service 4"); else printf("pthread_create successful for service 4\n");

  rt_param[4].sched_priority = rt_max_prio - 5;
  pthread_attr_setschedparam(&rt_sched_attr[4], &rt_param[4]);
  rc = pthread_create(&threads[4], &rt_sched_attr[4], Service_5, (void *)&threadParams[4]);
  if (rc < 0) perror("pthread_create for service 5"); else printf("pthread_create successful for service 5\n");

  rt_param[5].sched_priority = rt_max_prio - 6;
  pthread_attr_setschedparam(&rt_sched_attr[5], &rt_param[5]);
  rc = pthread_create(&threads[5], &rt_sched_attr[5], Service_6, (void *)&threadParams[5]);
  if (rc < 0) perror("pthread_create for service 6"); else printf("pthread_create successful for service 6\n");

  rt_param[6].sched_priority = rt_min_prio;
  pthread_attr_setschedparam(&rt_sched_attr[6], &rt_param[6]);
  rc = pthread_create(&threads[6], &rt_sched_attr[6], Service_7, (void *)&threadParams[6]);
  if (rc < 0) perror("pthread_create for service 7"); else printf("pthread_create successful for service 7\n");

  // --- Sequencer thread setup: pinned to core 1, RT_MAX priority ---
  printf("Start sequencer\n");
  sequencePeriods = 2000;

  CPU_ZERO(&threadcpu);
  CPU_SET(1, &threadcpu);   // Sequencer runs on core 1, per original design
  pthread_attr_init(&seq_sched_attr);
  pthread_attr_setinheritsched(&seq_sched_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&seq_sched_attr, SCHED_FIFO);
  pthread_attr_setaffinity_np(&seq_sched_attr, sizeof(cpu_set_t), &threadcpu);
  seq_param.sched_priority = rt_max_prio;
  pthread_attr_setschedparam(&seq_sched_attr, &seq_param);

  // --- Create and arm the timerfd: 100 Hz, i.e. 10 ms period ---
  timer_fd = timerfd_create(MY_TIMERFD_CLOCK, 0);
  if (timer_fd == -1) { perror("timerfd_create"); exit(-1); }

  struct itimerspec itime;
  itime.it_interval.tv_sec  = 0;
  itime.it_interval.tv_nsec = 10000000;  // 10 ms periodic reload
  itime.it_value.tv_sec     = 0;
  itime.it_value.tv_nsec    = 10000000;  // first expiry in 10 ms

  if (timerfd_settime(timer_fd, 0, &itime, NULL) == -1) {
    perror("timerfd_settime"); exit(-1);
  }

  // Sequencer now runs as a real thread, reading the timerfd in a loop,
  // instead of being invoked asynchronously as a SIGALRM handler.
  rc = pthread_create(&seq_thread, &seq_sched_attr, Sequencer, NULL);
  if (rc < 0) perror("pthread_create for sequencer");
  else printf("pthread_create successful for sequencer\n");

  pthread_join(seq_thread, NULL);

  for (i = 0; i < NUM_THREADS; i++) {
    if (pthread_join(threads[i], NULL) < 0) perror("main pthread_join");
    else printf("joined thread %d\n", i);
  }

  close(timer_fd);
  printf("\nTEST COMPLETE\n");
  return 0;
}

// Sequencer is now an ordinary thread function. It blocks on read()
// against the timerfd instead of being invoked by SIGALRM. Because this
// is normal thread context (not a signal handler), printf/syslog calls
// here are safe, and seqCnt/abort flags need no special atomic type --
// they are only ever touched from this one thread (writer) plus read-only
// checks elsewhere, same synchronization discipline as before.
void *Sequencer(void *arg) {
  struct timespec current_time_val;
  double current_realtime;
  uint64_t expirations;
  ssize_t s;

  while (!abortTest && seqCnt < sequencePeriods) {
    s = read(timer_fd, &expirations, sizeof(expirations));
    if (s != sizeof(expirations)) {
      perror("timerfd read");
      break;
    }

    if (expirations > 1) {
      // Overrun: one or more 10 ms periods were missed since the last
      // read. This condition is silently invisible with SIGALRM because
      // standard signals do not queue -- timerfd makes it observable.
      syslog(LOG_ERR, "Sequencer overrun: %llu missed periods\n",
             (unsigned long long)(expirations - 1));
    }

    seqCnt += expirations;

    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "Sequencer on core %d for cycle %llu @ sec=%6.9lf\n",
           sched_getcpu(), seqCnt, current_realtime - start_realtime);

    // Release each service at a sub-rate of the 100 Hz base rate --
    // identical modulus logic to the original.
    if ((seqCnt % 2)   == 0) sem_post(&semS1);   // Service_1 = 50 Hz
    if ((seqCnt % 5)   == 0) sem_post(&semS2);   // Service_2 = 20 Hz
    if ((seqCnt % 10)  == 0) sem_post(&semS3);   // Service_3 = 10 Hz
    if ((seqCnt % 20)  == 0) sem_post(&semS4);   // Service_4 = 5 Hz
    if ((seqCnt % 50)  == 0) sem_post(&semS5);   // Service_5 = 2 Hz
    if ((seqCnt % 100) == 0) sem_post(&semS6);   // Service_6 = 1 Hz
    if ((seqCnt % 100) == 0) sem_post(&semS7);   // Service_7 = 1 Hz
  }

  // Disarm the timer (equivalent to the original's zeroed itimerspec)
  struct itimerspec disarm = {{0, 0}, {0, 0}};
  timerfd_settime(timer_fd, 0, &disarm, NULL);

  printf("Disabling sequencer timerfd with abort=%d and %llu of %llu\n",
         abortTest, seqCnt, sequencePeriods);

  // Shutdown all services -- identical to original
  sem_post(&semS1); sem_post(&semS2); sem_post(&semS3); sem_post(&semS4);
  sem_post(&semS5); sem_post(&semS6); sem_post(&semS7);

  abortS1 = abortS2 = abortS3 = abortS4 = abortS5 = abortS6 = abortS7 = TRUE;

  pthread_exit((void *)0);
}

// --- Service_1..Service_7 are UNCHANGED from the original design ---
// They still block on sem_wait(), still check their own abort flag,
// still timestamp with clock_gettime and log via syslog. None of this
// needed to change because they were never signal handlers.

void *Service_1(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S1Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S1 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS1) {
    sem_wait(&semS1);
    S1Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S1 50 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S1Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_2(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S2Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S2 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS2) {
    sem_wait(&semS2);
    S2Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S2 20 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S2Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_3(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S3Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S3 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS3) {
    sem_wait(&semS3);
    S3Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S3 10 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S3Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_4(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S4Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S4 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS4) {
    sem_wait(&semS4);
    S4Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S4 5 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S4Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_5(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S5Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S5 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS5) {
    sem_wait(&semS5);
    S5Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S5 2 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S5Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_6(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S6Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S6 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS6) {
    sem_wait(&semS6);
    S6Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S6 1 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S6Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

void *Service_7(void *threadp) {
  struct timespec current_time_val; double current_realtime;
  unsigned long long S7Cnt = 0;
  clock_gettime(MY_CLOCK_TYPE, &current_time_val);
  current_realtime = realtime(&current_time_val);
  syslog(LOG_CRIT, "S7 thread @ sec=%6.9lf\n", current_realtime - start_realtime);
  while (!abortS7) {
    sem_wait(&semS7);
    S7Cnt++;
    clock_gettime(MY_CLOCK_TYPE, &current_time_val);
    current_realtime = realtime(&current_time_val);
    syslog(LOG_CRIT, "S7 1 Hz on core %d for release %llu @ sec=%6.9lf\n",
           sched_getcpu(), S7Cnt, current_realtime - start_realtime);
  }
  pthread_exit((void *)0);
}

double getTimeMsec(void) {
  struct timespec event_ts = {0, 0};
  clock_gettime(MY_CLOCK_TYPE, &event_ts);
  return ((event_ts.tv_sec) * 1000.0) + ((event_ts.tv_nsec) / 1000000.0);
}

double realtime(struct timespec *tsptr) {
  return ((double)(tsptr->tv_sec) + (((double)tsptr->tv_nsec) / 1000000000.0));
}

void print_scheduler(void) {
  int schedType = sched_getscheduler(getpid());
  switch (schedType) {
    case SCHED_FIFO:  printf("Pthread Policy is SCHED_FIFO\n");  break;
    case SCHED_OTHER: printf("Pthread Policy is SCHED_OTHER\n"); exit(-1); break;
    case SCHED_RR:    printf("Pthread Policy is SCHED_RR\n");    exit(-1); break;
    default:          printf("Pthread Policy is UNKNOWN\n");     exit(-1);
  }
}
