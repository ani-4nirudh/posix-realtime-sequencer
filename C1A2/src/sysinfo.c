#include "sysinfo.h"

int get_sys_info(struct utsname *ptr) {
  if ( uname(ptr) == -1 ) {
    syslog(LOG_ERR, "uname() failure: %s", strerror(errno));
    return -1;
  }

  return 0;
}
