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
 * @file sai_db_gen_utils.h
 *
 * @brief This file contains the function prototypes for the general
 *        utilities used to update the SQL DB tables.
 *************************************************************************/

#ifndef __SAI_DB_GEN_UTILS_H__
#define __SAI_DB_GEN_UTILS_H__

#include "saitypes.h"
#include <string>

/*
 * @brief Get the string for the SAI byte list.
 * @return comma seperated string for the given SAI byte list.
 */
std::string sai_vm_byte_list_str_get (const sai_u8_list_t *p_byte_list);

/*
 * @brief Get the string for the SAI object list.
 * @return comma seperated string for the given SAI object list.
 */
std::string sai_vm_obj_list_str_get (const sai_object_list_t *p_obj_list);

#endif /* __SAI_DB_GEN_UTILS_H__ */
