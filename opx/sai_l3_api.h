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
* @file sai_l3_api.h
*
* @brief This file contains the API signatures defined for SAI L3 component.
*
*************************************************************************/

#ifndef __SAI_L3_API_H__
#define __SAI_L3_API_H__

#include "std_type_defs.h"
#include "sai_l3_common.h"
#include "sai_oid_utils.h"
#include "sairouterinterface.h"
#include "saitypes.h"

/** \defgroup SAIL3NPUAPIs SAI - L3 NPU specific function implementations
 *  NPU specific functions for SAI L3 component
 *
 *  \{
 */

/**
 * @brief Create a virtual router in NPU.
 *
 * @param[in] p_vrf Pointer to the vrf node
 * @param[out] p_vr_id Pointer to Virtual Router ID
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vr_create_fn) (sai_fib_vrf_t *p_vrf,
                                              sai_npu_object_id_t *p_vr_id);

/**
 * @brief Remove a virtual router in NPU.
 *
 * @param[in] p_vrf   Pointer to the vrf node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vr_remove_fn) (sai_fib_vrf_t *p_vrf);

/**
 * @brief Sets an attribute to the Virtual Router in NPU.
 *
 * @param[in] p_vrf Pointer to the vrf node
 * @param[in] attr_flags Bit Mask of the attribute(s) that are to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vr_attribute_set_fn) (sai_fib_vrf_t *p_vrf,
                                                     uint_t attr_flags);

/**
 * @brief Get attributes from the virtual router in NPU.
 *
 * @param[in] p_vrf Pointer to the vrf node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vr_attribute_get_fn) (sai_fib_vrf_t *p_vrf,
                                                     uint_t attr_count,
                                                     sai_attribute_t *attr_list);

/**
 * @brief Create a router interface in NPU.
 *
 * @param[in] p_rif    Pointer to the router interface node
 * @param[out] p_id   Router interface index
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_router_interface_create_fn) (
                                             sai_fib_router_interface_t *p_rif,
                                             sai_npu_object_id_t *p_id);

/**
 * @brief Remove a router interface in NPU.
 *
 * @param[in] p_rif    Pointer to the router interface node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_router_interface_remove_fn) (
                                            sai_fib_router_interface_t *p_rif);


/**
 * @brief Sets an attribute to the router interface in NPU.
 *
 * @param[in] p_rif Pointer to the router interface node
 * @param[in] attr_flags Bit Mask of the attribute(s) that are to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_router_interface_attribute_set_fn) (
                                             sai_fib_router_interface_t *p_rif,
                                             uint_t attr_flags);

/**
 * @brief Gets attribute from the router interface in NPU.
 *
 * @param[in] p_rif Pointer to the router interface node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t
(*sai_npu_router_interface_attribute_get_fn) (sai_fib_router_interface_t *p_rif,
                                              uint_t attr_count,
                                              sai_attribute_t *attr_list);
/**
 * @brief Create a neighbor entry in NPU.
 *
 * @param[in] p_neighbor    Pointer to the neighbor entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_neighbor_create_fn) (sai_fib_nh_t *p_neighbor);

/**
 * @brief Remove a neighbor entry in NPU.
 *
 * @param[in] p_neighbor    Pointer to the neighbor entry node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_neighbor_remove_fn) (sai_fib_nh_t *p_neighbor);

/**
 * @brief Sets an attribute to the neighbor entry in NPU.
 *
 * @param[in] p_neighbor    Pointer to the neighbor entry node
 * @param[in] attr_flags Bit Mask of the attribute(s) that are to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_neighbor_attribute_set_fn) (
                                                     sai_fib_nh_t *p_neighbor,
                                                     uint_t attr_flags);

/**
 * @brief Get attributes for a neighbor entry
 *
 * @param[in] p_neighbor    Pointer to the neighbor entry node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_neighbor_attribute_get_fn) (
                                                 sai_fib_nh_t *p_neighbor,
                                                 uint_t attr_count,
                                                 sai_attribute_t *p_attr_list);

/**
 * @brief Create a next hop in NPU.
 *
 * @param[in] p_next_hop    Pointer to the next hop node
 * @param[out] p_next_hop_id   Next Hop index
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_create_fn) (sai_fib_nh_t *p_next_hop,
                                                    sai_npu_object_id_t *p_nh_id);

/**
 * @brief Remove a next hop in NPU.
 *
 * @param[in] p_next_hop    Pointer to the next hop node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_remove_fn) (sai_fib_nh_t *p_next_hop);

/**
 * @brief Get attributes for a Next Hop object
 *
 * @param[in] p_next_hop    Pointer to the next hop node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_attribute_get_fn) (
                                                 sai_fib_nh_t *p_next_hop,
                                                 uint_t attr_count,
                                                 sai_attribute_t *p_attr_list);

/**
 * @brief Create a next hop group in NPU.
 *
 * @param[in] p_group    Pointer to the next hop group node
 * @param[out] p_group_id   Next Hop Group index
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_group_create_fn) (
                                                sai_fib_nh_group_t *p_group,
                                                sai_npu_object_id_t *p_group_id);
/**
 * @brief Remove a next hop group in NPU.
 *
 * @param[in] p_group    Pointer to the next hop group node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_group_remove_fn) (
                                                sai_fib_nh_group_t *p_group);

/**
 * @brief Add next hop(s) to NH group in NPU.
 *
 * @param[in] p_group    Pointer to the next hop group node
 * @param[in] next_hop_count Number of next hops added to group
 * @param[in] ap_next_hop  Array of pointer to the next hop node(s)
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_add_to_group_fn) (
                                                  sai_fib_nh_group_t *p_group,
                                                  uint_t next_hop_count,
                                                  sai_fib_nh_t *ap_next_hop []);

/**
 * @brief Remove next hop(s) from NH group in NPU.
 *
 * @param[in] p_group    Pointer to the next hop group node
 * @param[in] next_hop_count  Number of next hops removed from group
 * @param[in] ap_next_hop  Array of pointer to the next hop node(s)
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_next_hop_remove_from_group_fn) (
                                                  sai_fib_nh_group_t *p_group,
                                                  uint_t next_hop_count,
                                                  sai_fib_nh_t *ap_next_hop []);

/**
 * @brief Create a route entry in NPU.
 *
 * @param[in] p_route   Pointer to route node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_route_create_fn) (sai_fib_route_t *p_route);

/**
 * @brief Remove a route entry in NPU.
 *
 * @param[in] p_route   Pointer to route node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_route_remove_fn) (sai_fib_route_t *p_route);

/**
 * @brief Set attributes for a route entry.
 *
 * @param[in] p_route   Pointer to route node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_route_attribute_set_fn) (
                                                sai_fib_route_t *p_route,
                                                uint_t attr_count,
                                                const sai_attribute_t *p_attr_list);

/**
 * @brief Get attributes for a route entry.
 *
 * @param[in] p_route   Pointer to route node
 * @param[in] attr_count Number of attributes
 * @param[inout] attr_list Attribute Id, Value pairs
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_route_attribute_get_fn) (
                                                sai_fib_route_t *p_route,
                                                uint_t attr_count,
                                                sai_attribute_t *p_attr_list);

/**
 * @brief Initialization of NPU specific L3 objects.
 *
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_fib_init_fn) (void);

/**
 * @brief API to set the Router MAC entry in NPU.
 *
 * @param[in] p_router_mac MAC address.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_fib_router_mac_set_fn) (
                                                const sai_mac_t *p_router_mac);

/*
 * @brief API to check if the virtual router attribute ID and value are valid
 * and supported in the NPU.
 *
 * @param[in] p_attr pointer to attribute ID, value pair.
 * @return SAI_STATUS_SUCCESS if the attribute is valid otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_vr_attr_validate_fn) (const sai_attribute_t *p_attr);

/*
 * @brief API to check if the router interface attribute ID and value are valid
 * in the NPU for the given router interface type.
 *
 * @param[in] type Router interface type which can be port or vlan.
 * @param[in] p_attr pointer to attribute ID, value pair.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_rif_attr_validate_fn) (sai_router_interface_type_t type,
                                                      const sai_attribute_t *p_attr);

/**
 * @brief API to set the ECMP max paths configuration in NPU.
 *
 * @param[in]ecmp_max_paths  ECMP max paths value.
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_ecmp_max_paths_set_fn) (uint_t ecmp_max_paths);

/**
 * @brief Updates the router interface setup on a LAG in NPU.
 *
 * @param[in] p_rif Pointer to the router interface node
 * @param[in] p_port_list Port object list for the list of member ports
 * @param[in] is_add Flag to indicate whether member(s) added or removed in the LAG
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_rif_lag_member_update_fn) (
                                       sai_fib_router_interface_t *p_rif,
                                       const sai_object_list_t *p_port_list,
                                       bool is_add);

/**
 * @brief Update the lpm route resolution for the Encap Next hop in NPU.
 *
 * @param[in] p_next_hop    Pointer to the Encap Next hop node
 * @param[in] p_underlay_route    Pointer to the Underlay route node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_encap_next_hop_route_resolve_fn) (
                                       sai_fib_nh_t *p_encap_nh,
                                       sai_fib_route_t *p_underlay_route);

/**
 * @brief Update the underlay neighbor resolution for the Encap Next hop in NPU.
 *
 * @param[in] p_encap_nh    Pointer to the Encap Next hop node
 * @param[in] p_neighbor   Pointer to the Underlay neighbor node
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_encap_next_hop_neighbor_resolve_fn) (
                                       sai_fib_nh_t *p_encap_nh,
                                       sai_fib_nh_t *p_neighbor);

/**
 * @brief Update the underlay neighbor attribute for the Encap Next hop in NPU.
 *
 * @param[in] p_encap_nh    Pointer to the Encap Next hop node
 * @param[in] p_neighbor Pointer to the Neighbor node
 * @param[in] neighbor_attr_info Pointer to the new attribute info for Neighbor
 * @param[in] attr_flags Bit Mask of the attribute(s) that are to be set
 * @return SAI_STATUS_SUCCESS if operation is successful otherwise a different
 *  error code is returned.
 */
typedef sai_status_t (*sai_npu_encap_next_hop_neighbor_attr_set_fn) (
                                             sai_fib_nh_t *p_encap_nh,
                                             sai_fib_nh_t *p_neighbor,
                                             sai_fib_nh_t *neighbor_attr_info,
                                             uint_t attr_flags);

/**
 * @brief L3 NPU Router API table.
 */
typedef struct _sai_npu_router_api_t {

    sai_npu_vr_create_fn           vr_create;
    sai_npu_vr_remove_fn           vr_remove;
    sai_npu_vr_attribute_set_fn    vr_attr_set;
    sai_npu_vr_attribute_get_fn    vr_attr_get;
    sai_npu_vr_attr_validate_fn    vr_attr_validate;
    sai_npu_fib_init_fn            fib_init;
    sai_npu_fib_router_mac_set_fn  router_mac_set;
    sai_npu_ecmp_max_paths_set_fn  ecmp_max_paths_set;
} sai_npu_router_api_t;

/**
 * @brief L3 NPU RIF API table.
 */
typedef struct _sai_npu_rif_api_t {
    sai_npu_router_interface_create_fn         rif_create;
    sai_npu_router_interface_remove_fn         rif_remove;
    sai_npu_router_interface_attribute_set_fn  rif_attr_set;
    sai_npu_router_interface_attribute_get_fn  rif_attr_get;
    sai_npu_rif_attr_validate_fn               rif_attr_validate;
    sai_npu_rif_lag_member_update_fn           rif_lag_member_update;
} sai_npu_rif_api_t;

/**
 * @brief L3 NPU Neighbor API table.
 */
typedef struct _sai_npu_neighbor_api_t {
    sai_npu_neighbor_create_fn          neighbor_create;
    sai_npu_neighbor_remove_fn          neighbor_remove;
    sai_npu_neighbor_attribute_set_fn   neighbor_attr_set;
    sai_npu_neighbor_attribute_get_fn   neighbor_attr_get;
} sai_npu_neighbor_api_t;

/**
 * @brief L3 NPU NextHop API table.
 */
typedef struct _sai_npu_nexthop_api_t {
    sai_npu_next_hop_create_fn                    nexthop_create;
    sai_npu_next_hop_remove_fn                    nexthop_remove;
    sai_npu_next_hop_attribute_get_fn             nexthop_attribute_get;
    sai_npu_encap_next_hop_route_resolve_fn       encap_nh_route_resolve;
    sai_npu_encap_next_hop_neighbor_resolve_fn    encap_nh_neighbor_resolve;
    sai_npu_encap_next_hop_neighbor_attr_set_fn   encap_nh_neighbor_attr_set;
} sai_npu_nexthop_api_t;

/**
 * @brief L3 NPU NH Group API table.
 */
typedef struct _sai_npu_nh_group_api_t {
    sai_npu_next_hop_group_create_fn           nh_group_create;
    sai_npu_next_hop_group_remove_fn           nh_group_remove;
    sai_npu_next_hop_add_to_group_fn           add_nh_to_group;
    sai_npu_next_hop_remove_from_group_fn      remove_nh_from_group;
} sai_npu_nh_group_api_t;

/**
 * @brief L3 NPU Route API table.
 */
typedef struct _sai_npu_route_api_t {
    sai_npu_route_create_fn         route_create;
    sai_npu_route_remove_fn         route_remove;
    sai_npu_route_attribute_set_fn  route_attr_set;
    sai_npu_route_attribute_get_fn  route_attr_get;
} sai_npu_route_api_t;

/**
 * \}
 */
#endif /* __SAI_L3_API_H__ */
