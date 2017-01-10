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
 * @file sai_vm_udf.h
 *
 * @brief This file contains the utilities and datastructure for SAI UDF
 *        NPU plugin functions in VM environment.
 *************************************************************************/

#ifndef __SAI_VM_UDF_H__
#define __SAI_VM_UDF_H__


#define SAI_VM_MAX_UDF_GROUP_NUMBER  (16)
#define SAI_VM_MAX_UDF_MATCH_NUMBER (512)
#define SAI_VM_MAX_UDF_NUMBER       (512)


typedef struct _sai_vm_udf_match_t {

    uint16_t  l2_type_data;
    uint16_t  l2_type_mask;
    uint8_t   l3_type_data;
    uint8_t   l3_type_mask;
    uint16_t  gre_type_data;
    uint16_t  gre_type_mask;
    uint8_t   priority;
    bool      udf_match_in_use;
} sai_vm_udf_match_t;


#endif /* __SAI_VM_UDF_H__ */
