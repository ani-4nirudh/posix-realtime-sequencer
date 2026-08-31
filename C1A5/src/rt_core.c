#include "rt_core.h"

sem_t g_sem[NUM_THREADS];

volatile int g_abort_services[NUM_THREADS] = {0};

struct timespec g_start_time;
double g_start_realtime; // Variable to store the time after conversion from timespec object

int g_timer_fd = -1;
unsigned long long int g_seq_cnt = 0;
unsigned long long int g_sequence_periods = 2000;
