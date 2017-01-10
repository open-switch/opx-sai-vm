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
 * @file sai_switch_db_api.h
 *
 * @brief This file contains the function prototypes for updating the
 *        SQL DB tables related to the SAI switch object attributes.
 *************************************************************************/

#ifndef __SAI_SWITCH_DB_API_H__
#define __SAI_SWITCH_DB_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "sai_oid_utils.h"
#include "saiswitch.h"
#include "saitypes.h"
#include "std_type_defs.h"

/*
 * @brief Switch DB attributes are grouped as multiple DB tables.
 *        Function pointer to initialize each switch DB table.
 */
typedef sai_status_t (*sai_vm_switch_db_table_init)(sai_switch_id_t switch_id);

/*
 * @brief Create switch DB tables and initialize the fields in the switch DB
 * tables with default values.
 * @return SAI status code.
 */
sai_status_t sai_vm_switch_db_init (sai_switch_id_t switch_id);

/*
 * @brief Set the switch attribute in the switch DB table entry.
 * @param switch_id - SAI switch ID.
 * @param p_attr - pointer to SAI switch attribute ID, value pair.
 * @return SAI status code.
 */
sai_status_t sai_switch_attribute_set_db_entry (sai_switch_id_t switch_id,
                                                const sai_attribute_t *p_attr);

/*
 * @brief Get the switching mode for the switch.
 * @param p_mode - pointer to the switching mode.
 * @return SAI status code.
 */
sai_status_t sai_npu_switching_mode_get (sai_switch_switching_mode_t *p_mode);
#ifdef __cplusplus
}
#endif

#endif /* __SAI_SWITCH_DB_API_H__ */
