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
 * @file sai_vm_udf.h
 *
 * @brief This file contains the utilities and datastructure for SAI UDF
 *        NPU plugin functions in VM environment.
 */

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
