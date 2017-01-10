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
 * @file sai_vm_db_utils.h
 *
 * @brief This file contains the function prototypes for the utilities
 *        and operations on the SQL DB tables related to the SAI objects
 *        in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_DB_UTILS_H__
#define __SAI_VM_DB_UTILS_H__

#include "saitypes.h"
#include "db_sql_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Initialize DB schema for SAI tables and get handle for operating on
 * the DB.
 * @return sai status code
 */
sai_status_t sai_vm_db_init (void);

/*
 * @brief Get the Database Handle to perform various operations
 * @return sqlite database handle
 */
db_sql_handle_t sai_vm_get_db_handle (void);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_VM_DB_UTILS_H__ */
