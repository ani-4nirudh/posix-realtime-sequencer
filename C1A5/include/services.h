/**************************************************************************
 * Copyright (C) 2026 by Anirudh Singh
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Anirudh Singh is not liable for any misuse of this material.
 *************************************************************************/

/**
 * @file services.h
 * @brief Header file for services.c
 *
 * @description
 * Each thread instance would run its own generic_service() function. Tracking the service release for each thread is performed using the gloabl sempahore variables.
 *
 * @author Anirudh Singh
 * @date 20th August 2026
 */

#ifndef SERVICES_H
#define SERVICES_H

/**
 * @brief Function passed to each service during thread creation using pthread_create() 
 *
 * @param thread_ctx_ptr Pointer to the thread_ctx object
 * 
 * @detailed The function waits for the semaphore from the sequencer based on the tick count. Each service thread has its own semaphore variable from the global array g_sem
 */
void *generic_service(void *thread_ctx_ptr);

#endif /* !__SERVICES_H__
#define __SERVICES_H__ */
