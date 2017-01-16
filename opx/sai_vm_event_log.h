/************************************************************************
 * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************/
/*
 * @file sai_vm_event_log.h
 *
 * @brief This file contains the Event logging definitions for SAI
 *        objects in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_EVENT_LOG_H__
#define __SAI_VM_EVENT_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif
#include "event_log.h"
#include "event_log_types.h"
#include "sai_event_log.h"
#ifdef __cplusplus
}
#endif

/** Logging utility for SAI Scheduler API */
#define SAI_VM_DB_LOG(level, msg, ...) \
        SAI_LOG_UTIL(ev_log_t_DB_SQL, level, msg, ##__VA_ARGS__); \

/** Per log level based macros for SAI Scheduler API */
#define SAI_VM_DB_LOG_TRACE(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_VM_DB_LOG_CRIT(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_VM_DB_LOG_ERR(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_VM_DB_LOG_INFO(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_VM_DB_LOG_WARN(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_VM_DB_LOG_NTC(msg, ...) \
            SAI_VM_DB_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

#endif /* __SAI_VM_EVENT_LOG_H__ */
