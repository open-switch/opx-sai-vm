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
 * @file sai_vm_db_utils.h
 *
 * @brief This file contains the function prototypes for the utilities
 *        and operations on the SQL DB tables related to the SAI objects
 *        in VM environment.
 */

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
