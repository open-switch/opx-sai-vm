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
* @file sai_l3_api_utils.h
*
* @brief This file contains macro definitions and function prototypes
* used in the SAI L3 API implementation.
*
*************************************************************************/
#ifndef __SAI_L3_API_UTILS_H__
#define __SAI_L3_API_UTILS_H__

#include "saiswitch.h"
#include "sairouterinterface.h"
#include "saitypes.h"
#include "saistatus.h"
#include "std_struct_utils.h"
#include "std_bit_masks.h"
#include "std_type_defs.h"
#include "sai_l3_common.h"
#include "sai_l3_util.h"
#include "sai_fdb_common.h"
#include "sai_lag_callback.h"
#include "sai_oid_utils.h"
#include "sai_common_utils.h"

/*
 * Virtual Router functionality related macros.
 */
#define SAI_FIB_VRF_MAX_ATTR_COUNT         (5)
#define SAI_FIB_VRF_MANDATORY_ATTR_COUNT   (0)

#define SAI_FIB_RDX_MAX_NAME_LEN           (64)

/*
 * Router Interface functionality related macros.
 */
#define SAI_FIB_RIF_MAX_ATTR_COUNT         (8)
#define SAI_FIB_RIF_MANDATORY_ATTR_COUNT   (2)
#define SAI_FIB_RIF_TYPE_MAX               (SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)
#define SAI_FIB_RIF_TYPE_NONE              (SAI_FIB_RIF_TYPE_MAX + 1)

/*
 * Route functionality related macros.
 */
#define SAI_FIB_ROUTE_MAX_ATTR_COUNT       (4)
#define SAI_FIB_ROUTE_DFLT_PKT_ACTION      (SAI_PACKET_ACTION_FORWARD)
#define SAI_FIB_ROUTE_DFLT_TRAP_PRIO       (0)

#define SAI_FIB_ROUTE_TREE_KEY_SIZE \
        (sizeof (sai_fib_route_key_t) * BITS_PER_BYTE)

#define SAI_FIB_MAX_DEP_ROUTES_WALK_COUNT  (0)

#define SAI_FIB_READ_FD    (0)
#define SAI_FIB_WRITE_FD   (1)
#define SAI_FIB_MAX_FD     (2)

static inline uint_t sai_fib_route_key_len_get (uint_t prefix_len)
{
    return (((STD_STR_SIZE_OF(sai_ip_address_t, addr_family)) * BITS_PER_BYTE)
            + prefix_len);
}

/* Util function to update the next hop node owner_flag field */
static inline void sai_fib_set_next_hop_owner (sai_fib_nh_t *p_next_hop,
                                               sai_fib_nh_owner_flag flag)
{
    if (p_next_hop) {

        p_next_hop->owner_flag |= (0x1 << flag);
    }
}

static inline void sai_fib_reset_next_hop_owner (sai_fib_nh_t *p_next_hop,
                                                 sai_fib_nh_owner_flag flag)
{
    if (p_next_hop) {

        p_next_hop->owner_flag &= ~(0x1 << flag);
    }
}

static inline bool sai_fib_is_default_route_node (sai_fib_route_t *p_route)
{
    return ((sai_fib_is_ip_addr_zero (&p_route->key.prefix)) &&
            (p_route->prefix_len == 0));
}

static inline void sai_fib_incr_nh_ref_count (sai_fib_nh_t *p_nh_node)
{
    p_nh_node->ref_count++;
}

static inline void sai_fib_decr_nh_ref_count (sai_fib_nh_t *p_nh_node)
{
    if (p_nh_node->ref_count > 0) {
        p_nh_node->ref_count--;
    }
}

static inline void sai_fib_incr_nh_group_ref_count(sai_fib_nh_group_t *p_grp_node)
{
    p_grp_node->ref_count++;
}

static inline void sai_fib_decr_nh_group_ref_count(sai_fib_nh_group_t *p_grp_node)
{
    if (p_grp_node->ref_count > 0) {
        p_grp_node->ref_count--;
    }
}

sai_status_t sai_fib_next_hop_ip_address_validate (
                                            const sai_ip_address_t *p_ip_addr);

sai_status_t sai_fib_next_hop_rif_set (sai_fib_nh_t *p_nh_info,
                                       const sai_attribute_value_t *p_value,
                                       bool is_create_req);

sai_fib_nh_t *sai_fib_ip_next_hop_node_create (sai_object_id_t vrf_id,
                                               sai_fib_nh_t *p_nh_info,
                                               sai_status_t *p_status);

sai_status_t sai_fib_ip_next_hop_create (sai_fib_nh_t *p_nh_info,
                                        sai_fib_nh_t **p_out_next_hop_node);

sai_status_t sai_fib_ip_next_hop_remove (sai_fib_nh_t *p_nh_node);

sai_status_t sai_fib_check_and_delete_ip_next_hop_node (
                                            sai_object_id_t vrf_id,
                                            sai_fib_nh_t *p_nh_node);

void sai_fib_next_hop_log_error (sai_fib_nh_t *p_next_hop,
                                 const char *p_error_str);

void sai_fib_next_hop_log_trace (sai_fib_nh_t *p_next_hop,
                                 const char *p_trace_str);

sai_status_t sai_fib_vrf_rif_list_update (
                                 sai_fib_router_interface_t *p_rif_node,
                                 bool is_add);

sai_status_t sai_router_ecmp_max_paths_set (uint_t max_paths);

sai_status_t sai_rif_lag_callback (sai_object_id_t lag_id,
                                   sai_lag_operation_t lag_operation,
                                   const sai_object_list_t *port_list);

sai_status_t sai_neighbor_fdb_callback (uint_t num_upd,
                                        sai_fdb_internal_notification_data_t *fdb_upd);

sai_status_t sai_fib_get_vr_id_for_rif (sai_object_id_t rif_id,
                                        sai_object_id_t *vr_id);

bool sai_fib_route_is_nh_info_match (sai_fib_route_t *p_route_1,
                                     sai_fib_route_t *p_route_2);

void sai_fib_encap_next_hop_log_trace (sai_fib_nh_t *p_encap_nh,
                                       const char *p_trace_str);
void sai_fib_encap_next_hop_log_error (sai_fib_nh_t *p_encap_nh,
                                       const char *p_error_str);
sai_status_t sai_fib_encap_next_hop_tunnel_id_set (sai_fib_nh_t *p_nh_info,
                                                   uint_t attr_index,
                                                   const sai_attribute_t *p_attr);
sai_status_t sai_fib_encap_next_hop_create (sai_fib_nh_t *p_encap_nh,
                                            sai_npu_object_id_t *hw_id);
sai_status_t sai_fib_encap_next_hop_remove (sai_fib_nh_t *p_encap_nh);

void sai_fib_encap_nh_dep_route_add (sai_fib_route_t *p_route);
void sai_fib_encap_nh_dep_route_remove (sai_fib_route_t *p_route);

sai_status_t sai_fib_internal_default_route_node_del (sai_fib_vrf_t *p_vrf_node,
                                                      sai_ip_addr_family_t af);
sai_status_t sai_fib_internal_default_route_node_add (sai_fib_vrf_t *p_vrf_node,
                                                      sai_ip_addr_family_t af);

void sai_fib_route_affected_encap_nh_update (sai_fib_route_t *p_route,
                                             dn_sai_operations_t op_code);
void sai_fib_route_attr_set_affected_encap_nh_update (
                                           sai_fib_route_t *p_route_node,
                                           sai_fib_route_t *p_new_route_info);
void sai_fib_neighbor_affected_encap_nh_resolve (sai_fib_nh_t *p_neighbor,
                                                 dn_sai_operations_t op_type);
void sai_fib_neighbor_dep_encap_nh_list_update (sai_fib_nh_t *p_neighbor,
                                                sai_fib_nh_t *p_attr_info,
                                                uint_t attr_flags);
void sai_fib_nh_group_dep_encap_nh_update (sai_fib_nh_group_t *p_nh_group,
                                           sai_fib_nh_t *p_underlay_nh,
                                           bool is_add);
sai_status_t sai_fib_encap_nh_dep_route_walker_create (void);

sai_status_t sai_fib_lag_rif_mapping_insert (sai_object_id_t lag_id,
                                             sai_object_id_t rif_id);

sai_status_t sai_fib_lag_rif_mapping_remove (sai_object_id_t lag_id,
                                             sai_object_id_t rif_id);

sai_status_t sai_fib_get_rif_id_from_lag_id (sai_object_id_t lag_id,
                                             sai_object_id_t *rif_id);

void sai_fib_dump_vr (sai_object_id_t vr_id);

void sai_fib_dump_all_vr (void);

void sai_fib_dump_rif (sai_object_id_t rif_id);

void sai_fib_dump_all_rif_in_vr (sai_object_id_t vr_id);

void sai_fib_dump_all_rif (void);

void sai_fib_dump_all_route_in_vr (sai_object_id_t vrf);

void sai_fib_dump_nh (sai_object_id_t nh_id);

void sai_fib_dump_all_nh (void);

void sai_fib_dump_all_neighbor_in_vr (sai_object_id_t vrf);

void sai_fib_dump_nh_group (sai_object_id_t group_id);

void sai_fib_dump_all_nh_group (void);

void sai_fib_dump_nh_list_from_nh_group (sai_object_id_t group_id);

void sai_fib_dump_neighbor_mac_entry_tree (void);

#endif /* __SAI_L3_API_UTILS_H__ */
