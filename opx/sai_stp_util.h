/*
 * Copyright (c) 2018 Dell Inc.
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
* @file sai_stp_util.h
*
* @brief This file contains the utility functions for SAI STP component.
*
*************************************************************************/
#ifndef __SAI_STP_UTIL_H__
#define __SAI_STP_UTIL_H__

#include "sai_event_log.h"

/** \defgroup SAISTPUTILS SAI - STP Utility functions
 *  Util functions in the SAI STP component
 *
 *  \{
 */

/** Logging utility for SAI STP API */
#define SAI_STP_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_STP, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_STP, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI STP API */
#define SAI_STP_LOG_TRACE(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_STP_LOG_CRIT(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_STP_LOG_ERR(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_STP_LOG_INFO(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_STP_LOG_WARN(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_STP_LOG_NTC(msg, ...) \
        SAI_STP_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/**
 * \}
 */

#endif /* __SAI_STP_UTIL_H__ */
