/************************************************************************
 * * LEGALESE:   "Copyright (c) 2015, Dell Inc. All rights reserved."
 * *
 * * This source code is confidential, proprietary, and contains trade
 * * secrets that are the sole property of Dell Inc.
 * * Copy and/or distribution of this source code or disassembly or reverse
 * * engineering of the resultant object code are strictly forbidden without
 * * the written consent of Dell Inc.
 * *
 * ************************************************************************/
/**
 * * @file sai_port_breakout_utils.h
 * *
 * * @brief This file contains prototype declarations for utility functions
            used to set and get breakout modes for ports
 * *
 * *************************************************************************/
#ifndef SAI_PORT_BREAKOUT_UTILS_H
#define SAI_PORT_BREAKOUT_UTILS_H

extern "C" {
#include "saiswitch.h"
#include "saitypes.h"
#include "saiport.h"
#include "saistatus.h"
}

/**************************************************************************
 *                     Function Prototypes
 **************************************************************************/

bool sai_port_type_logical(sai_object_id_t port,
                           sai_port_api_t*
                           sai_port_api_table);

sai_status_t sai_port_break_out_mode_set(sai_switch_api_t *sai_switch_api_table,
                                        sai_port_api_t *sai_port_api_table,
                                        sai_object_id_t port,
                                        sai_port_breakout_mode_type_t
                                        breakout_mode);

sai_status_t sai_port_break_in_mode_set(sai_switch_api_t *sai_switch_api_table,
                                        sai_port_api_t *sai_port_api_table,
                                        uint32_t port_count,
                                        sai_object_id_t *port_list,
                                        sai_port_breakout_mode_type_t
                                        breakout_mode);

sai_status_t sai_port_break_out_mode_get(sai_port_api_t *sai_port_api_table,
                                         sai_object_id_t port,
                                         sai_port_breakout_mode_type_t
                                         breakout_mode);

sai_status_t sai_check_supported_breakout_mode(sai_port_api_t *sai_port_api_table,
                                                      sai_object_id_t port,
                                                      sai_port_breakout_mode_type_t
                                                      breakout_mode,
                                                      bool *is_supported);
#endif
