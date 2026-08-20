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
 * @brief Header file containing the constants used for this project
 *
 * @author Anirudh Singh
 * @date 17th August 2026
 *
 */

#ifndef __APP_CONFIG_H__
#define __APP_CONFIG_H__

/**
 * This is a constant which will be prefixed to the syslog messages
 *
 */
#define COURSE_PREFIX "[COURSE:1][ASSIGNMENT:4]"
#define NUM_THREADS 128
#define CORE_NUM 2 // CPU core used for core pinning

#endif /* !__APP_CONFIG_H__                                                    \
##define __APP_CONFIG_H__ */
