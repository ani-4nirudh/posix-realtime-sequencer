/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file sysinfo.h
 * @brief Header file for src/sysinfo.c
 *
 * @author Anirudh Singh
 * @date 11th August 2026
 *
 */

#ifndef __SYSINFO_H__
#define __SYSINFO_H__

#include <sys/utsname.h>

/**
 * @brief Get system information using uname function
 *
 * @param ptr Pointer to the utsname struct
 *
 * @detailed This function will take pointer to the utsname struct object.
 * The struct object would then be populated with the required fields.
 */
int get_sys_info(struct utsname *ptr);

#endif /* !__SYSINFO_H__                                                       \
#define __SYSINFO_H__ */
