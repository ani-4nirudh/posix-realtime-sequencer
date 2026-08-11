#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <sys/utsname.h>
#include <stdio.h>
#include <pthread.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>

#define COURSE_PREFIX "[COURSE:1][ASSIGNMENT:2]"

int get_sys_info(struct utsname *ptr);

#endif /* !__SYSINFO_H__
#define __SYSINFO_H__ */
