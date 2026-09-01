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
 * @brief Contains the macros and tick counts for each service as an array THREAD_RATES.
 *
 * @description
 * The element inside THREAD_RATES is the amount of counts after which their corresponding service will be released. Each timer tick is of duration BASE_PERIOD_NS (i.e. 10ms). For e.g. the total time elapsed after 100 ticks is 100 * 10 = 1000 ms. Therefore, the service[0] would be started every second.
 * 
 * Similarly, service[4] (the last thread) would be released every 10 s.
 *
 * Total duration of the sequencer is: TOTAL_TICKS * BASE_PERIOD_NS = 2000 * 10 ms = 20 s.
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

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
