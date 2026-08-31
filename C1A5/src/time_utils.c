#include "time_utils.h"

double get_realtime(struct timespec *tp) {
  double nano_to_sec = 1000 * 1000 * 1000;
  double tp_sec = (double) tp->tv_sec;
  double tp_nsec = (double) tp->tv_nsec;
  double ret = tp_sec + (tp_nsec / nano_to_sec);
  
  return ret;
}
