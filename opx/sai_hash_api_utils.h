/************************************************************************
* LEGALESE:   "Copyright (c); 2016, Dell Inc. All rights reserved."
*
* This source code is confidential, proprietary, and contains trade
* secrets that are the sole property of Dell Inc.
* Copy and/or distribution of this source code or disassembly or reverse
* engineering of the resultant object code are strictly forbidden without
* the written consent of Dell Inc.
*
************************************************************************/
/**
* @file sai_hash_api_utils.h
*
* @brief This private header file contains util functions and function
*        prototypes used by SAI Hash API functions
*
****************************************************************************/

#ifndef _SAI_HASH_API_UTILS_H_
#define _SAI_HASH_API_UTILS_H_

#include "saistatus.h"
#include "saitypes.h"
#include <stdlib.h>

/* Max value for the software object index */
#define DN_SAI_MAX_HASH_INDEX 256

static inline dn_sai_hash_object_t *dn_sai_hash_obj_node_alloc (void)
{
    return ((dn_sai_hash_object_t *) calloc (1, sizeof (dn_sai_hash_object_t)));
}

static inline int32_t *dn_sai_hash_native_field_list_alloc (uint_t count)
{
    return ((int32_t *) calloc (count, sizeof (int32_t)));
}

static inline sai_object_id_t *dn_sai_hash_udf_group_list_alloc (uint_t count)
{
    return ((sai_object_id_t *) calloc (count, sizeof (sai_object_id_t)));
}

static inline void dn_sai_hash_obj_node_free (dn_sai_hash_object_t *p_hash_obj)
{
    free ((void *) p_hash_obj->native_fields_list.list);
    free ((void *) p_hash_obj->udf_group_list.list);
    free ((void *) p_hash_obj);
}

static inline void dn_sai_hash_udf_group_list_free (
                                             dn_sai_hash_object_t *p_hash_obj)
{
    free ((void *) p_hash_obj->udf_group_list.list);
}

static inline void dn_sai_hash_native_field_list_free (
                                             dn_sai_hash_object_t *p_hash_obj)
{
    free ((void *) p_hash_obj->native_fields_list.list);
}

static inline bool dn_sai_hash_is_obj_used (dn_sai_hash_object_t *p_hash_obj)
{
    return (p_hash_obj->ref_count ? true : false);
}

sai_status_t dn_sai_switch_hash_attr_set (sai_attr_id_t attr_id,
                                          sai_object_id_t hash_obj_id);
sai_status_t dn_sai_switch_hash_attr_get (sai_attribute_t *attr);



#endif /* _SAI_HASH_API_UTILS_H_ */


