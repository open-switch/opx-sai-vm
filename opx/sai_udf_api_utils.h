/************************************************************************
* LEGALESE:   "Copyright (c); 2015, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_udf_api_utils.h
*
* @brief This private header file contains SAI UDF util functions and
*        data structure definitions
*
****************************************************************************/

#ifndef _SAI_UDF_API_UTILS_H_
#define _SAI_UDF_API_UTILS_H_

#include "saitypes.h"
#include "saiudf.h"
#include "sai_common_utils.h"
#include "sai_oid_utils.h"
#include "sai_udf_common.h"
#include "std_rbtree.h"

/**
 * @brief Data structure for the SAI UDF global parameters.
 *
 */
typedef struct _dn_sai_udf_global_t {

    /** Container for UDF Group objects. Nodes of type dn_sai_udf_group_t */
    rbtree_handle    udf_group_tree;

    /** Container for UDF objects. Nodes of type dn_sai_udf_t */
    rbtree_handle    udf_obj_tree;

    /** Flag to indicate if global params are initialized */
    bool             is_init_complete;
} dn_sai_udf_global_t;

void dn_sai_udf_lock();
void dn_sai_udf_unlock();

sai_udf_api_t *sai_udf_api_query (void);
void dn_sai_udf_group_api_fill (sai_udf_api_t *p_udf_api_table);
void dn_sai_udf_match_api_fill (sai_udf_api_t *p_udf_api_table);
void dn_sai_udf_api_fill (sai_udf_api_t *p_udf_api_table);

dn_sai_udf_global_t *dn_sai_udf_access_global_param (void);
sai_status_t dn_sai_udf_attr_list_validate (sai_object_type_t obj_type,
                                            uint32_t attr_count,
                                            const sai_attribute_t *attr_list,
                                            dn_sai_operations_t type);
dn_sai_udf_t *dn_sai_udf_node_get (sai_object_id_t udf_id);
dn_sai_udf_group_t *dn_sai_udf_group_node_get (sai_object_id_t udf_group_id);
dn_sai_udf_t *dn_sai_udf_group_get_first_udf_node (dn_sai_udf_group_t *p_udf_group);
dn_sai_udf_t *dn_sai_udf_group_get_next_udf_node (dn_sai_udf_group_t *p_udf_group,
                                                  dn_sai_udf_t *p_udf_node);
sai_status_t dn_sai_udf_group_hw_id_get (sai_object_id_t udf_group_id,
                                         sai_npu_object_id_t *hw_id);

sai_status_t dn_sai_udf_group_type_get (sai_object_id_t udf_group_id,
                                        sai_udf_group_type_t *type);


/*
 * Accessor Function for UDF group tree handle
 */
static inline rbtree_handle dn_sai_udf_group_tree_handle (void)
{
    return (dn_sai_udf_access_global_param ()->udf_group_tree);
}

/*
 * Accessor Function for UDF tree handle
 */
static inline rbtree_handle dn_sai_udf_tree_handle (void)
{
    return (dn_sai_udf_access_global_param ()->udf_obj_tree);
}

#endif /* _SAI_UDF_API_UTILS_H_ */


