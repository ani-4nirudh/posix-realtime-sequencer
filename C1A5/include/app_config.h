#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#define NUM_THREADS 5
#define COURSE_PREFIX "[COURSE:1][ASSIGNMENT:5]"

#define EVEN_THREAD_CORE 2
#define ODD_THREAD_CORE 3
#define SEQUENCER_CORE 1

#define MY_CLOCK_TYPE CLOCK_MONOTONIC_RAW
#define TIMERFD_CLOCK_TYPE CLOCK_MONOTONIC

#define BASE_PERIOD_NS 10000000ULL // 10 ms or 100 Hz
#define TOTAL_TICKS 2000ULL

static const unsigned int THREAD_RATES[NUM_THREADS] = {
    100, // 1s
    200, // 2s
    400, // 4s
    500, // 5s
    1000 // 10s
};

#endif /* !APP_CONFIG_H
#define APP_CONFIG_H */
