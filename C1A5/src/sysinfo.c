/**
 * @file sysinfo.c
 * @brief Function description for get_sys_info()
 *
 * @author Anirudh Singh
 * @date 11th August 2026
 *
 */

#include <errno.h>
#include <string.h>
#include <syslog.h>

#include "sysinfo.h"

int get_sys_info(struct utsname *ptr) {
  if (uname(ptr) == -1) {
    syslog(LOG_ERR, "uname() failure: %s", strerror(errno));
    return -1;
  }

  return 0;
}
