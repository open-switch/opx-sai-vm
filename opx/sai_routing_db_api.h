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
 * @file sai_routing_db_api.h
 *
 * @brief This file contains the function prototypes for updating the
 *        SQL DB tables related to the SAI routing objects in VM
 *        environment.
 */

#ifndef __SAI_ROUTING_DB_API_H__
#define __SAI_ROUTING_DB_API_H__

#include "sai_l3_common.h"
#include "sairoute.h"
#include "saineighbor.h"
#include "sainexthopgroup.h"
#include "saitypes.h"
#include "db_sql_ops.h"
#include "std_type_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * @brief Create a new entry in the Neighbor database table
 * @param p_neighbor - New neighbor node to be inserted in DB.
 * @return sai status code
 */
sai_status_t sai_neighbor_create_db_entry (sai_fib_nh_t *p_neighbor);

/*
 * @brief Delete an entry from Neighbor database table
 * @param p_neighbor - Neighbor node to be deleted from DB.
 * @return sai status code
 */
sai_status_t sai_neighbor_delete_db_entry (sai_fib_nh_t *p_neighbor);

/*
 * @brief Set attributes of an entry in Neighbor database table
 * @param p_neighbor - Neighbor node with cached attribute info.
 * @param attr_flags - Bitmap indicating attribute(s) to be set.
 * @return sai status code
 */
sai_status_t sai_neighbor_set_db_entry (sai_fib_nh_t *p_neighbor,
                                        uint_t attr_flags);

/*
 * @brief Create a new entry in the nexthop database table
 * @param nh_id - Next Hop id
 * @param p_next_hop - Next Hop node with cached attribute info.
 * @return sai status code
 */
sai_status_t sai_nexthop_create_db_entry (sai_object_id_t nh_id,
                                          sai_fib_nh_t *p_next_hop);

/*
 * @brief Delete an entry from nexthop database table
 * @param nh_id - next hop id to be deleted
 * @return sai status code
 */
sai_status_t sai_nexthop_delete_db_entry (sai_object_id_t nh_id);

/*
 * @brief Create a new entry in the nexthop group database table
 * @param nh_grp_id - Next Hop Group id.
   @param type - Next Hop group type.
 * @param nh_count - Number of Next Hops added on creation.
 * @param ap_next_hop - List of Next Hop nodes added on creation.
 * @return sai status code
 */
sai_status_t sai_nh_group_create_db_entry (sai_object_id_t nh_grp_id,
                                           sai_next_hop_group_type_t type,
                                           uint_t nh_count,
                                           sai_fib_nh_t *ap_next_hop []);

/*
 * @brief Delete an existing entry from nexthop group database table
 * @param nh_grp_id - Next Hop Group id.
 * @return sai status code
 */
sai_status_t sai_nh_group_delete_db_entry (sai_object_id_t nh_grp_id);

/*
 * @brief Update the list of Next Hops added to the Next Hop Group on the
 * Next Hop Group List Database table.
 * @param nh_grp_id - Next Hop Group id.
 * @param nh_count_added - Number of Next Hops added to existing next hop list.
 * @param ap_next_hop - List of Next Hop nodes added to existing next hop list.
 * @param total_nh_count - Updated total Next Hop count.
 * Next Hops.
 * @return sai status code
 */
sai_status_t sai_nh_group_add_nh_list_to_db_entry (sai_object_id_t nh_grp_id,
                                                   uint_t nh_count_added,
                                                   sai_fib_nh_t *ap_next_hop [],
                                                   uint_t total_nh_count);

/*
 * @brief Update the list of Next Hops deleted from the Next Hop Group on the
 * Next Hop Group List Database table.
 * @param nh_grp_id - Next Hop Group id.
 * @param nh_count_deleted - Number of Next Hops deleted from existing next hop
 * list.
 * @param ap_next_hop - List of Next Hop nodes deleted from existing next hop
 * list.
 * @param total_nh_count - Updated total Next Hop count.
 * @return sai status code
 */
sai_status_t sai_nh_group_delete_nh_list_from_db_entry (
sai_object_id_t nh_grp_id, uint_t nh_count_deleted,
sai_fib_nh_t *ap_next_hop [], uint_t total_nh_count);

/*
 * @brief Create a new entry in the route database table
 * @param p_route - New route node to be inserted in DB.
 * @return sai status code
 */
sai_status_t sai_route_create_db_entry (sai_fib_route_t *p_route);
/*
 * @brief Delete an entry from route database table
 * @param p_route - Route node to be deleted.
 * @return sai status code
 */
sai_status_t sai_route_delete_db_entry (sai_fib_route_t *p_route);

/*
 * @brief Set attributes of an entry in route database table
 * @param p_route - Route node with cached attribute info.
 * @param attr_flag - attribute to be set.
 * @return sai status code
 */
sai_status_t sai_route_set_db_entry (sai_fib_route_t *p_route,
                                     uint_t attr_flag);

/*
 * @brief Create a new entry in the router database table
 * @param vrf_id - virtual router id.
 * @param p_vrf_node - VRF node with cached attribute info.
 * @return sai status code
 */
sai_status_t sai_router_create_db_entry (sai_object_id_t vrf_id,
                                         sai_fib_vrf_t *p_vrf_node);

/*
 * @brief Delete an entry from router database table
 * @param vrf_id - a virtual router id to be deleted
 * @return sai status code
 */
sai_status_t sai_router_delete_db_entry (sai_object_id_t vrf_id);

/*
 * @brief Set attributes of an entry in router database table
 * @param p_vrf_node - VRF node with cached attribute info.
 * @param attr_flag - Bitmap indicating attribute(s) to be set.
 * @return sai status code
 */
sai_status_t sai_router_set_db_entry (sai_fib_vrf_t *p_vrf, uint_t attr_flag);

/*
 * @brief Create a new entry in the routerintf database table
 * @param rif_id - Router interface id
 * @param p_rif - Router interface node with cached attribute info.
 * @return sai status code
 */
sai_status_t sai_routerintf_create_db_entry (sai_object_id_t rif_id,
                                             sai_fib_router_interface_t *p_rif);

/*
 * @brief Delete an entry from routerintf database table
 * @param rif_id - router interface id to be deleted
 * @return sai status code
 */
sai_status_t sai_routerintf_delete_db_entry (sai_object_id_t rif_id);

/*
 * @brief Set attributes of an entry in routerintf database table
 * @param p_rif - Router interface node with cached attribute info.
 * @param attr_flag - Bitmap indicating attribute(s) to be set.
 * @return sai status code
 */
sai_status_t sai_routerintf_set_db_entry (sai_fib_router_interface_t *p_rif,
                                          uint_t attr_flag);

#ifdef __cplusplus
}
#endif

#endif /* __SAI_ROUTING_DB_API_H__ */
