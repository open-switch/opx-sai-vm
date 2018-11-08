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
/**
* @file sai_acl_type_defs.h
*
* @brief This file contains the datastructure definitions for SAI ACL objects.
*
*************************************************************************/
#ifndef _SAI_ACL_TYPE_DEFS_H_
#define _SAI_ACL_TYPE_DEFS_H_

#include "sai_samplepacket_defs.h"
#include "sai_oid_utils.h"

#include "saiacl.h"
#include "saiextensions.h"

#include "std_type_defs.h"
#include "std_rbtree.h"
#include "std_llist.h"


/** ACL Table Identifier Starting Index */
#define SAI_ACL_TABLE_ID_MIN 0x1

/** ACL Table Identifier Last Index */
#define SAI_ACL_TABLE_ID_MAX 0x20

/** ACL Table Invalid Identifier */
#define SAI_ACL_INVALID_TABLE_ID 0

/** ACL Rule Default Admin State */
#define SAI_ACL_RULE_DEFAULT_ADMIN_STATE 2

typedef enum _sai_bcm_acl_app_table_indexes
{
    SAI_ACL_INGRESS_SYSTEM_FLOW = 0,
    SAI_ACL_INGRESS_OPENFLOW,
    SAI_ACL_INGRESS_VLT,
    SAI_ACL_INGRESS_ISCSI,
    SAI_ACL_INGRESS_FCOE,
    SAI_ACL_INGRESS_FCOE_FPORT,
    SAI_ACL_INGRESS_FEDGOV,
    SAI_ACL_INGRESS_L2_ACL,
    SAI_ACL_INGRESS_V4_ACL,
    SAI_ACL_INGRESS_V6_ACL,
    SAI_ACL_INGRESS_V4_PBR,
    SAI_ACL_INGRESS_V6_PBR,
    SAI_ACL_INGRESS_L2_QOS,
    SAI_ACL_INGRESS_V4_QOS,
    SAI_ACL_INGRESS_V6_QOS,
    SAI_ACL_EGRESS_L2_ACL,
    SAI_ACL_EGRESS_V4_ACL,
    SAI_ACL_EGRESS_V6_ACL,
    /** If any new application gets added in policy table
     * add equivalent index in this table on the need basis */
    SAI_ACL_GROUP_END
}sai_bcm_acl_app_table_indexes;

/**
 * @brief SAI ACL Table ID generator Data Structure
 *
 * Contains the generated table Id and the table usage
 */
typedef struct _sai_acl_table_id_node_t {

    /** Flag to indicate whether table is in use */
    bool table_in_use;

    /** Table Id generated during SAI ACL module initialization */
    uint_t table_id;
} sai_acl_table_id_node_t;

typedef struct _sai_acl_table_static_config_t {

    /** ACL table application static priority */
    uint_t priority[SAI_ACL_GROUP_END];

    uint_t max_ifp_slice;

    uint_t max_efp_slice;

    uint_t *ifp_slice_depth_list;

    uint_t *efp_slice_depth_list;

    /**In Tomohawk, non-single wide mode takes 2 entry space*/
    uint_t  depth_per_entry;
} sai_acl_table_static_config_t;

/**
 * @brief SAI ACL Node Data Structure
 *
 * Contains SAI ACL trees required for different ACL attributes
 */
typedef struct _sai_acl_node_t {

    /** Nodes of type sai_acl_table_t */
    rbtree_handle sai_acl_table_tree;

    /** Nodes of type sai_acl_rule_t */
    rbtree_handle sai_acl_rule_tree;

    /** Nodes of type sai_acl_counter_t */
    rbtree_handle sai_acl_counter_tree;

    /** Nodes of type sai_acl_table_group_t */
    rbtree_handle sai_acl_table_group_tree;

    /** Nodes of type sai_acl_table_group_member_t */
    rbtree_handle sai_acl_table_group_member_tree;

    /** Nodes of type sai_acl_range_t */
    rbtree_handle sai_acl_range_tree;

    /** Nodes of type sai_acl_slice_t */
    rbtree_handle sai_acl_slice_tree;

    /** Acl table static priorities */
    sai_acl_table_static_config_t sai_acl_table_config;
} sai_acl_node_t, *acl_node_pt;

/**
 * @brief SAI ACL Attribute Value
 *
 * Contains the union of ACL attribute data types
 */
typedef struct _sai_acl_attr_value_t {

    /** Boolean Data Type */
    bool booldata;

    /** 8-bit Data Type */
    sai_uint8_t u8;

    /** 16-bit Data Type */
    sai_uint16_t u16;

    /** 32-bit Data Type */
    sai_uint32_t u32;

    /** Enum Data Type */
    sai_int32_t s32;

    /** MAC Data Type */
    sai_mac_t mac;

    /** IPv4 Address Data Type */
    sai_ip4_t ip4;

    /** IPv6 Address Data Type */
    sai_ip6_t ip6;

    /** Object Id Data Type */
    sai_object_id_t oid;

    /** Object List Data Type */
    sai_object_list_t obj_list;

    /** 8-bit Data List Type */
    sai_u8_list_t u8_list;

} sai_acl_attr_value_t;

/**
 *  @brief SAI ACL UDF Data Structure
 *
 *  Contains SAI ACL UDF specific information
 */
typedef struct _sai_acl_udf_field_t {

    /** UDF Group Object Id */
    sai_object_id_t         udf_group_id;

    /** Index to UDF Attribute Range */
    sai_attr_id_t           udf_attr_index;

    /** UDF Group NPU Id */
    sai_npu_object_id_t     udf_npu_id;

} sai_acl_udf_field_t;

/**
 * @brief SAI ACL Filter Data Structure
 *
 * Contains SAI ACL Filter specific information to add qualifier
 */
typedef struct _sai_acl_filter_t {

    /** ACL Filter Attributes */
    bool field_change;
    bool new_field;

    /** Field status */
    bool enable;

    /** Field Specifier */
    sai_acl_entry_attr_t field;

    /** Field match mask */
    sai_acl_attr_value_t match_mask;

    /** Expected AND result using match mask above with packet field value */
    sai_acl_attr_value_t match_data;

    /** UDF specific information */
    sai_acl_udf_field_t  udf_field;
} sai_acl_filter_t;

/**
 * @brief SAI ACL Action Data Structure
 *
 * Contains SAI ACL Action specific information to add action
 */
typedef struct _sai_acl_action_t {

    /** ACL Action Attributes */
    bool action_change;
    bool new_action;

    /** Action Specifier */
    sai_acl_entry_attr_t action;

    /** Action enable/disable */
    bool enable;

    /** Action parameter */
    sai_acl_attr_value_t parameter;
} sai_acl_action_t;

/**
 * @brief SAI ACL Table Key
 *
 * Contains the SAI ACL Table Key
 */
typedef struct _sai_acl_table_key_t {

    /** ACL Table Identifier */
    sai_object_id_t  acl_table_id;
} sai_acl_table_key_t;

/**
 * @brief SAI ACL Table Data Structure
 *
 * Contains the SAI ACL Table Attributes.
 */
typedef struct _sai_acl_table_t {

    /** Table Key to ACL Table RB Tree */
    sai_acl_table_key_t     table_key;

    /** ACL Table Attributes */
    uint64_t                acl_table_priority;
    sai_acl_stage_t         acl_stage;
    uint_t                  table_size;
    sai_object_id_t         table_group_id;
    bool                    virtual_group_create;

    /** Count of Fields in the Field List */
    uint_t                  field_count;

    /** List of Fields */
    sai_acl_table_attr_t    *field_list;

    /** Count of Actions in the Action List */
    uint_t                  action_count;

    /** List of Actions */
    sai_acl_action_type_t    *action_list;

    /** Count of UDF Group Fields */
    uint_t                  udf_field_count;

    /** List of UDF Group Fields */
    sai_acl_udf_field_t     *udf_field_list;

    /** Count of Rules present in the ACL table */
    uint_t                  rule_count;

    /** Number of counters associated with ACL table */
    uint_t                  num_counters;

    /** ACL Rule list head. Nodes of type sai_acl_rule_t */
    std_dll_head            rule_head;

    /** Contains ACL Table related NPU specific information */
    void                    *npu_table_info;
} sai_acl_table_t;

/**
 * @brief SAI ACL Rule Key
 *
 * Contains the key for SAI ACL Rule
 */
typedef struct _sai_acl_rule_key_t {

    /** ACL Rule Identifier*/
    sai_object_id_t      acl_id;
} sai_acl_rule_key_t;

/**
 * @brief SAI ACL Rule Data Structure
 *
 * Contains the SAI ACL Rule Attributes.
 */
typedef struct _sai_acl_rule_t {

    /** Link to the ACL Rule list head in ACL Table data structure */
    std_dll                 rule_link;

    /** Rule Key to ACL Rule RB Tree */
    sai_acl_rule_key_t      rule_key;

    /** ACL Rule Priority */
    uint_t                  acl_rule_priority;

    /** ACL Table Id to which rule is associated */
    sai_object_id_t         table_id;

    /** ACL Rule Enabled/Disabled */
    uint_t                  acl_rule_state;

    /** Count of filters present in filter list */
    uint_t                  filter_count;

    /** ACL Filter list holding filter specific information */
    sai_acl_filter_t        *filter_list;

    /** Count of actions present in action list */
    uint_t                  action_count;

    /** ACL Action list holding action specific information */
    sai_acl_action_t        *action_list;

    /** Contains ACL Rule related NPU specific information */
    void                    *npu_rule_info;

    /** Link to the policer associated with acl rule */
    std_dll                 policer_glue;

    /** ACL Counter Identifier attached to the ACL Rule */
    sai_object_id_t         counter_id;

    /** SamplePacket session attached to ACL */
    sai_object_id_t         samplepacket_id[SAI_SAMPLEPACKET_DIR_MAX];

    /** Policer id attached to ACL */
    sai_object_id_t         policer_id;
} sai_acl_rule_t;

/**
 * @brief SAI ACL Counter Type Enum
 *
 * Type of Counter supported to attach to ACL Rule
 */
typedef enum _sai_acl_counter_type_t {

    /** Count based on bytes */
    SAI_ACL_COUNTER_BYTES,

    /** Count based on packets */
    SAI_ACL_COUNTER_PACKETS,

    /** Count based on both bytes and packets */
    SAI_ACL_COUNTER_BYTES_PACKETS
} sai_acl_counter_type_t;

/**
 * @brief SAI ACL Counter Key
 *
 * Contains the key for SAI ACL Counter
 */
typedef struct _sai_acl_counter_key_t {

    /** ACL Counter Identifier */
    sai_object_id_t  counter_id;
} sai_acl_counter_key_t;

/**
 * @brief SAI ACL Counter Data Structure
 *
 * Contains the SAI ACL Counter Attributes.
 */
typedef struct _sai_acl_counter_t {

      /** Counter Key to ACL Counter RB Tree */
      sai_acl_counter_key_t        counter_key;

      /** ACL Table to which counter is associated */
      sai_object_id_t              table_id;

      /** Counter Type */
      sai_acl_counter_type_t       counter_type;

      /** Number of ACL rules which share this counter */
      uint32_t                     shared_count;

      /** Contains ACL Counter related NPU specific information */
      void                         *npu_counter_info;
} sai_acl_counter_t;


typedef struct _sai_acl_table_group_t {

    /** Key to the acl group tree */
    sai_object_id_t  acl_table_group_id;

    /** Acl stage ingress/egress sai_acl_stage_t */
    sai_acl_stage_t  acl_stage;

    /** List of points where the group/table is to be applied
     * Includes port,lag,vlan,rif and switch */
    sai_s32_list_t  acl_bind_list;

    /** Lookup type parallel/sequential */
    sai_acl_table_group_type_t acl_group_type;

    /** List of the ACL tables part of this group */
    std_dll_head   table_list_head;

    /** Number of tables in the group */
    uint_t table_count;

    /** Reference count */
    int ref_count;
} sai_acl_table_group_t;


typedef struct _sai_acl_table_group_member_t {

    /** Link to group */
    std_dll  member_link;
    /** Key to the group_memeber tree */
    sai_object_id_t  acl_table_group_member_id;

    /** group id that this table id is part of */
    sai_object_id_t  acl_group_id;

    /** table id which is part of the acl group */
    sai_object_id_t  acl_table_id;

    /** Priority of the acl table */
    sai_uint32_t     priority;

    /** Reference count */
    int ref_count;
} sai_acl_table_group_member_t;

typedef struct _sai_acl_range_t {

    /** Key to sai_acl_range tree */
    sai_object_id_t      acl_range_id;

    /** Type of range */
    sai_acl_range_type_t range_type;

    /** Min and max limits of range */
    sai_s32_range_t     range_limit;

    /** Npu range information */
    void  *npu_range_info;

    /** reference count */
    int ref_count;
} sai_acl_range_t;

/**
 * @brief SAI ACL Slice ACL table Data Structure
 *
 * Contains the SAI ACL Slice ACL table info Attributes.
 */
typedef struct _sai_acl_slice_acl_table_t {

    /** Link to the next ACL table info sharing the same slice */
    std_dll                 acl_table_link;
    /** Object ID of the ACL table present in the current slice */
    sai_object_id_t         acl_table_object_id;

}sai_acl_slice_acl_table_t;

typedef struct _sai_acl_slice_t {
    /** Key to sai_acl_slice_tree */
    sai_object_id_t acl_slice_id;

    /** Slice id */
    sai_uint32_t    slice_id;

    /** Pipe id */
    sai_uint32_t    pipe_id;

    /** Stage */
    sai_acl_stage_t acl_stage;

    /** Free entries in the current slice */
    sai_uint32_t    slice_depth;
} sai_acl_slice_t;
#endif /* _SAI_ACL_TYPE_DEFS_H_ */
