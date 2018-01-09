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
/**
* @file sai_samplepacket_util.h
*
* @brief This file contains the utility functions for SAI Samplepacket component.
*
*************************************************************************/
#ifndef __SAI_SAMPLEPACKET_UTIL_H__
#define __SAI_SAMPLEPACKET_UTIL_H__

#include "sai_event_log.h"

/** \defgroup SAISAMPLEPACKETUTILS SAI - Samplepacket Utility functions
 *  Util functions in the SAI Samplepacket component
 *
 *  \{
 */

/** Logging utility for SAI Samplepacket API */
#define SAI_SAMPLEPACKET_LOG(level, msg, ...) \
    do { \
        if (sai_is_log_enabled (SAI_API_SAMPLEPACKET, level)) { \
            SAI_LOG_UTIL(ev_log_t_SAI_SAMPLEPACKET, level, msg, ##__VA_ARGS__); \
        } \
    } while (0)

/** Per log level based macros for SAI Samplepacket API */
#define SAI_SAMPLEPACKET_LOG_TRACE(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#define SAI_SAMPLEPACKET_LOG_CRIT(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_CRITICAL, msg, ##__VA_ARGS__)

#define SAI_SAMPLEPACKET_LOG_ERR(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)

#define SAI_SAMPLEPACKET_LOG_INFO(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_INFO, msg, ##__VA_ARGS__)

#define SAI_SAMPLEPACKET_LOG_WARN(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_WARN, msg, ##__VA_ARGS__)

#define SAI_SAMPLEPACKET_LOG_NTC(msg, ...) \
        SAI_SAMPLEPACKET_LOG (SAI_LOG_LEVEL_NOTICE, msg, ##__VA_ARGS__)

/**
 * \}
 */

#endif /* __SAI_SAMPLEPACKET_UTIL_H__ */
