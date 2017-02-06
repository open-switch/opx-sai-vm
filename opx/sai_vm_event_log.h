/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file sai_vm_event_log.h
 *
 * @brief This file contains the Event logging definitions for SAI
 *        objects in VM environment.
 */

#ifndef __SAI_VM_EVENT_LOG_H__
#define __SAI_VM_EVENT_LOG_H__

#include "event_log.h"
#include "event_log_types.h"
#include "sai_event_log.h"

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
