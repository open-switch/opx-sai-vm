/************************************************************************
 * LEGALESE:   Copyright (c) 1999-2014, Dell Inc
 *
 * This source code is confidential, proprietary, and contains trade
 * secrets that are the sole property of Dell Inc.
 * Copy and/or distribution of this source code or disassembly or reverse
 * engineering of the resultant object code are strictly forbidden without
 * the written consent of Dell Inc.
 *
 ************************************************************************
 */
/*!
 * \file   sai_event_log.h
 * \brief  SAI Event logging functions
 */

#ifndef _SAI_EVENT_LOG_H_
#define _SAI_EVENT_LOG_H_

#include "sai.h"
#include "sai_gen_utils.h"
#include "event_log.h"
#include "event_log_types.h"

/**
 * @brief  sub-levels needed by EV_LOG framework
 */
#define SAI_TRACE_SUBLVL 2  /* MINOR sub-level */
#define SAI_ERR_SUBLVL   1  /* MAJOR sub-level */

/**
 * @brief  Generic SAI log level specific logging functions
 */
#define SAI_LOG_LEVEL_DEBUG(mod, ID, msg, ...) \
                 EV_LOG_TRACE(mod, SAI_TRACE_SUBLVL, ID, msg, ##__VA_ARGS__)

#define SAI_LOG_LEVEL_INFO(mod, ID, msg, ...) \
                 EV_LOG_INFO(mod, SAI_TRACE_SUBLVL, ID, msg, ##__VA_ARGS__)

/* @TODO Map to the EV_LOG macro that sets the LOG_NOTICE level */
#define SAI_LOG_LEVEL_NOTICE(mod, ID, msg, ...) \
                 EV_LOG_INFO(mod, SAI_TRACE_SUBLVL, ID, msg, ##__VA_ARGS__)

/* @TODO Map to the EV_LOG macro that sets the LOG_WARNING level */
#define SAI_LOG_LEVEL_WARN(mod, ID, msg, ...) \
                 EV_LOG_INFO(mod, SAI_TRACE_SUBLVL, ID, msg, ##__VA_ARGS__)

#define SAI_LOG_LEVEL_ERROR(mod, ID, msg, ...) \
                 EV_LOG_ERR(mod, SAI_ERR_SUBLVL, ID, msg, ##__VA_ARGS__)

/* @TODO Map to the EV_LOG macro that sets the LOG_CRIT level */
#define SAI_LOG_LEVEL_CRITICAL(mod, ID, msg, ...) \
                 EV_LOG_ERR(mod, SAI_ERR_SUBLVL, ID, msg, ##__VA_ARGS__)

/**
 * @brief  Generic SAI logging util function
 */
#define SAI_LOG_UTIL(MOD, LEVEL, msg, ...) \
                     LEVEL(MOD, sai_switch_id_str_get(), msg, ##__VA_ARGS__)
#endif /* _SAI_EVENT_LOG_H_ */
