
/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file sysinfo.c
 * @brief Source file for sysinfo.h
 *
 * @description
 *
 * @author Anirudh Singh
 * @date 20th August 2026
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
