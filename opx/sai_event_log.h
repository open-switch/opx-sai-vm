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

/*
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
