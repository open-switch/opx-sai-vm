/************************************************************************
* * LEGALESE:   "Copyright (c) 2017, Dell Inc. All rights reserved."
* *
* * This source code is confidential, proprietary, and contains trade
* * secrets that are the sole property of Dell Inc.
* * Copy and/or distribution of this source code or disassembly or reverse
* * engineering of the resultant object code are strictly forbidden without
* * the written consent of Dell Inc.
* *
************************************************************************/
/***
 * \file    sai_ipmc_api.h
 *
 * \brief Declaration of SAI IPMC related APIs
*/

#if !defined (__SAIIPMCAPI_H_)
#define __SAIIPMCAPI_H_
#include "saitypes.h"
#include "saistatus.h"
#include "sai_ipmc_common.h"
#include "sai_l3_mcast_common.h"

/** SAI IPMC API - Init IPMC Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_ipmc_tree_init(void);

/** SAI IPMC API - Lock for accessing IPMC tree
*/
void sai_ipmc_lock(void);

/** SAI IPMC API - API to release the IPMC lock
*/
void sai_ipmc_unlock(void);

/** SAI IPMC API - Insert IPMC group node to the tree
    \param[in] ipmc_group_info ipmc group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE
*/
sai_status_t sai_add_ipmc_group_node(dn_sai_ipmc_group_node_t *ipmc_group_info);

/** SAI IPMC API - Remove IPMC group node from the tree
    \param[in] ipmc_group_info ipmc group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_OBJECT_IN_USE, SAI_STATUS_INVALID_PARAMETER
            SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_ipmc_group_node(dn_sai_ipmc_group_node_t *ipmc_group_info);

/** SAI IPMC API - Find ipmc Group node from the tree
    \param[in] ipmc group object id
    \return A Valid pointer to the ipmc group node in the tree else NULL
*/
dn_sai_ipmc_group_node_t * sai_find_ipmc_group_node(sai_object_id_t ipmc_group_id);

/** SAI IPMC API - Add ipmc member node to the member tree
    \param[in] ipmc_member_node_t member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE, SAI_STATUS_ITEM_NOT_FOUND
                     SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_add_ipmc_group_member_node(dn_sai_ipmc_group_member_node_t *ipmc_group_member_info);

/** SAI IPMC API - Find ipmc Group Member node from the tree
    \param[in] ipmc Group Member object id
    \return A Valid pointer to the ipmc Group Member node in the tree else NULL
 */
dn_sai_ipmc_group_member_node_t* sai_find_ipmc_group_member_node( sai_object_id_t ipmc_group_member_id);

/** SAI IPMC API - Remove ipmc group member node from the member tree
    \param[in] ipmc_group_member_node_t ipmc group member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_INVALID_PORT_MEMBER, SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_ipmc_group_member_node(dn_sai_ipmc_group_member_node_t *ipmc_group_member_info);

/** SAI IPMC API - Find the ipmc group member node for the ipmc group and Router Interface
    \param[in] ipmc_group_node ipmc group node
    \param[in] sai_object_id_t rtr intf object id
    \return A Valid pointer to the ipmc group member node or NULL
*/
dn_sai_ipmc_group_member_dll_node_t* sai_find_ipmc_group_member_node_from_router_intf(
        dn_sai_ipmc_group_node_t *ipmc_group_node, sai_object_id_t rtr_intf);

/** SAI IPMC API - Get the list of Router Interfaces from the ipmc group
    \param[in] ipmc_group_node ipmc group node
    \param[in] sai_object_list_t List for filling the Router Interface members
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_BUFFER_OVERFLOW
*/
sai_status_t sai_ipmc_group_rtr_intf_list_get(dn_sai_ipmc_group_node_t *ipmc_group_node,
        sai_object_list_t *ipmc_rtr_intf_list);

/** SAI IPMC API - Get the list of ports within the Router Interface from the ipmc group
    \param[in] ipmc_group_node ipmc group node
    \param[in] ipmc_rtr_intf ipmc router interface
    \param[in] sai_object_list_t List for filling the Router Interface port members
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_BUFFER_OVERFLOW
*/
sai_status_t sai_ipmc_group_rtr_intf_port_list_get(dn_sai_ipmc_group_node_t *ipmc_group_node,
        sai_object_id_t *ipmc_rtr_intf, sai_object_list_t *ipmc_rtr_intf_port_list);

/** SAI IPMC API - Get Next IPMC Group
    \param[in] ipmc_group_node Current IPMC group node
    \return Success: A Valid pointer to IPMC group node in cache
            Failure: NULL
*/
dn_sai_ipmc_group_node_t *sai_ipmc_group_get_next(dn_sai_ipmc_group_node_t *l2mc_group_node);

/** SAI IPMC API - Get Next IPMC Group Member
    \param[in] ipmc_group_member_node Current IPMC Group Member node
    \return Success: A Valid pointer to IPMC Group Member node in cache
            Failure: NULL
*/

dn_sai_ipmc_group_member_node_t *sai_ipmc_group_member_get_next(dn_sai_ipmc_group_member_node_t *ipmc_group_member_node);

/** SAI IPMC API - Insert IPMC RPF group node to the tree
    \param[in] ipmc_rpf_group_info ipmc rpf group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE
*/
sai_status_t sai_add_ipmc_rpf_group_node(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_info);

/** SAI IPMC API - Remove IPMC RPF group node from the tree
    \param[in] ipmc_rpf_group_info ipmc rpf group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_OBJECT_IN_USE, SAI_STATUS_INVALID_PARAMETER
            SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_ipmc_rpf_group_node(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_info);

/** SAI IPMC API - Find ipmc RPF Group node from the tree
    \param[in] ipmc rpf group object id
    \return A Valid pointer to the ipmc rpf group node in the tree else NULL
*/
dn_sai_ipmc_rpf_group_node_t * sai_find_ipmc_rpf_group_node(sai_object_id_t ipmc_rpf_group_id);

/** SAI IPMC API - Add ipmc rpf group member node to the member tree
    \param[in] ipmc_member_node_t member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE, SAI_STATUS_ITEM_NOT_FOUND
                     SAI_STATUS_NO_MEMORY
*/
sai_status_t sai_add_ipmc_rpf_group_member_node(dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_info);

/** SAI IPMC API - Find ipmc RPF Group Member node from the tree
    \param[in] ipmc RPF Group Member object id
    \return A Valid pointer to the ipmc RPF Group Member node in the tree else NULL
 */
dn_sai_ipmc_rpf_group_member_node_t *sai_find_ipmc_rpf_group_member_node(sai_object_id_t ipmc_rpf_group_member_id);

/** SAI IPMC API - Remove ipmc rpf group member node from the member tree
    \param[in] ipmc_rpf_group_member_node_t ipmc rpf group member node info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_INVALID_PORT_MEMBER, SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_ipmc_rpf_group_member_node(dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_info);

/** SAI IPMC API - Find the ipmc rpf group member node for the ipmc rpf group and Router Interface
    \param[in] ipmc_rpf_group_node rpf group node
    \param[in] sai_object_id_t rtr intf object id
    \return A Valid pointer to the ipmc rpf group member node or NULL
*/
dn_sai_ipmc_rpf_group_member_dll_node_t* sai_find_ipmc_rpf_group_member_node_from_router_intf(
        dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node, sai_object_id_t rtr_intf);

/** SAI IPMC API - Get the list of Router Interfaces from the ipmc rpf group
    \param[in] ipmc_rpf_group_node ipmc group node
    \param[in] sai_object_list_t List for filling the Router Interface members
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_BUFFER_OVERFLOW
*/
sai_status_t sai_ipmc_rpf_group_rtr_intf_list_get(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        sai_object_list_t *ipmc_rtr_intf_list);

/** SAI IPMC API - Get the list of ports within the Router Interface from the ipmc rpf group
    \param[in] ipmc_rpf_group_node ipmc rpf group node
    \param[in] ipmc_rpf_rtr_intf ipmc rpf router interface
    \param[in] sai_object_list_t List for filling the Router Interface port members
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_BUFFER_OVERFLOW
*/
sai_status_t sai_ipmc_rpf_group_rtr_intf_port_list_get(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node,
        sai_object_id_t *ipmc_rpf_rtr_intf, sai_object_list_t *ipmc_rpf_rtr_intf_port_list);

/** SAI IPMC API - Insert IPMC Repl group node to the tree
    \param[in] ipmc_repl_group_info ipmc repl group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_FAILURE
*/
sai_status_t sai_add_ipmc_repl_group_node(dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_info);

/** SAI IPMC API - Find ipmc Replication Group node from the tree
    \param[in] ipmc repl group object id
    \return A Valid pointer to the ipmc repl group node in the tree else NULL
*/
dn_sai_ipmc_repl_group_node_t * sai_find_ipmc_repl_group_node(sai_object_id_t ipmc_repl_group_id);

/** SAI IPMC API - Remove IPMC Replication group node from the tree
    \param[in] ipmc_repl_group_info ipmc repl group info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_OBJECT_IN_USE, SAI_STATUS_INVALID_PARAMETER
            SAI_STATUS_ITEM_NOT_FOUND
*/
sai_status_t sai_remove_ipmc_repl_group_node(dn_sai_ipmc_repl_group_node_t *ipmc_repl_group_info);

/** SAI IPMC API - Get Next IPMC Group
    \param[in] ipmc_group_node Current IPMC group node
    \return Success: A Valid pointer to IPMC group node in cache
            Failure: NULL
*/
dn_sai_ipmc_group_node_t *sai_ipmc_group_get_next(dn_sai_ipmc_group_node_t *l2mc_group_node);

/** SAI IPMC API - Get Next IPMC Group Member
    \param[in] ipmc_group_member_node Current IPMC Group Member node
    \return Success: A Valid pointer to IPMC Group Member node in cache
            Failure: NULL
*/

dn_sai_ipmc_group_member_node_t *sai_ipmc_group_member_get_next(dn_sai_ipmc_group_member_node_t *ipmc_group_member_node);

/** SAI IPMC API - Get Next IPMC RPF Group
    \param[in] ipmc_rpf_group_node Current IPMC rpf group node
    \return Success: A Valid pointer to IPMC rpf group node in cache
            Failure: NULL
*/
dn_sai_ipmc_rpf_group_node_t *sai_ipmc_rpf_group_get_next(dn_sai_ipmc_rpf_group_node_t *ipmc_rpf_group_node);

/** SAI IPMC API - Get Next IPMC RPF Group Member
    \param[in] ipmc_rpf_group_member_node Current IPMC rpf Group Member node
    \return Success: A Valid pointer to IPMC rpf Group Member node in cache
            Failure: NULL
*/
dn_sai_ipmc_rpf_group_member_node_t *sai_ipmc_rpf_group_member_get_next(dn_sai_ipmc_rpf_group_member_node_t *ipmc_rpf_group_member_node);

/** SAI IPMC API - Get Next IPMC Replication Group
    \param[in] ipmc_repl_group_node Current IPMC replication Group node
    \return Success: A Valid pointer to IPMC replication Group node in cache
            Failure: NULL
*/
dn_sai_ipmc_repl_group_node_t* sai_ipmc_repl_group_get_next(dn_sai_ipmc_repl_group_node_t* ipmc_repl_group_node);

/** SAI L3 MCAST API - Init L3 MCAST Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_l3_mcast_init(void);

/** SAI L3 IPMC API - Init L3 IPMC RPF Group Module data structures
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_UNINITIALIZED
*/
sai_status_t sai_ipmc_rpf_group_init(void);

/** SAI L3 MCAST API - Lock for accessing L3 Mcast tree
*/
void sai_l3_mcast_lock(void);
/** SAI L3 MCAST API - API to release the L3 mcast lock
*/
void sai_l3_mcast_unlock(void);
/** SAI L3 MCAST API - Insert l3 multicast entry to the cache
    \param[in] l3_mcast_entry_node L3 MCAST entry info
    \return Success: SAI_STATUS_SUCCESS
            Failure: SAI_STATUS_ITEM_ALREADY_EXISTS, SAI_STATUS_FAILURE
*/
sai_status_t sai_insert_l3_mcast_entry_node(dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);
/** SAI L3 MCAST API - Find l3 multicast entry in the cache
    \param[in] l3_mcast_entry_node L3 MCAST entry info
    \return A Valid pointer to the l3 mcast entry node in the cache else NULL
*/
dn_sai_l3_mcast_entry_node_t * sai_find_l3_mcast_entry(dn_sai_l3_mcast_entry_node_t *entry_node);
/** SAI L3 MCAST API - Get the next l3 multicast entry in the cache
    \param[in] l3_mcast_key Key of the current L3 MCAST entry node
    \return A Valid pointer to the mcast entry node in the cache else NULL
*/
dn_sai_l3_mcast_entry_node_t *sai_get_next_l3_mcast_entry_node (dn_sai_l3_mcast_entry_key_t *l3_mcast_key);
/** SAI L3 MCAST API - Remove l3 multicast entry from the cache
    \param[in] A valid l3 mcast_entry_node pointer in the cache
    \return Success: SAI_STATUS_SUCCESS
*/
sai_status_t sai_remove_l3_mcast_entry_node(dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node);

/* Utility routines to maintain a map of RtrIntf ObjID, IPMC/RPFGroupObjID <----> IPMC Entry */
sai_status_t sai_port_to_rtf_intf_ipmcobj_map_insert (sai_object_id_t lag_port_id,
                                                sai_object_id_t rtf_intf_id, sai_object_id_t ipmc_obj_id);

sai_status_t sai_port_to_rtf_intf_ipmcobj_map_remove (sai_object_id_t lag_port_id,
                                                sai_object_id_t rtf_intf_id, sai_object_id_t ipmc_obj_id);

sai_status_t sai_port_to_rtf_intf_ipmcobj_map_list_get (sai_object_id_t  lag_port_id,
                                               uint_t          *count,
                                               sai_object_id_t *rtf_intf_list,
                                               sai_object_id_t *ipmc_obj_list);

sai_status_t sai_port_to_rtf_intf_ipmcobj_map_count (sai_object_id_t  lag_port_id,
                                                uint_t          *p_out_count);

sai_status_t sai_port_to_rtf_intf_ipmcobj_map_at_index (sai_object_id_t lag_port_id,
                                                   uint_t          index,
                                                   sai_object_id_t *rtf_intf_id,
                                                   sai_object_id_t *ipmc_obj_id);

sai_status_t sai_rtf_intf_ipmcobj_to_ipmcentry_map_insert ( sai_object_id_t rtf_intf_id, sai_object_id_t ipmc_obj_id,
                                                            sai_object_id_t ipmc_entry);

sai_status_t sai_rtf_intf_ipmcobj_to_ipmcentry_map_remove ( sai_object_id_t rtf_intf_id, sai_object_id_t ipmc_obj_id,
                                                             sai_object_id_t ipmc_entry);
sai_status_t sai_rtf_intf_ipmcobj_to_ipmcentry_map_list_get ( sai_object_id_t rtf_intf_id,
                                               sai_object_id_t ipmc_obj_id,
                                               uint_t          *count,
                                               sai_object_id_t  *ipmc_entry_list);

sai_status_t sai_rtf_intf_ipmcobj_to_ipmcentry_map_count (sai_object_id_t  rtf_intf_id,
                                                sai_object_id_t  ipmc_obj_id,
                                                uint_t          *p_out_count);

sai_status_t sai_rtf_intf_ipmcobj_to_ipmcentry_map_at_index ( sai_object_id_t rtf_intf_id,
                                                   sai_object_id_t ipmc_obj_id,
                                                   uint_t          index,
                                                   sai_object_id_t *ipmc_entry);

void sai_ipmc_rpf_group_dump_all(void);

void sai_ipmc_rpf_group_dump(sai_object_id_t ipmc_rpf_group_obj);

void sai_ipmc_group_dump_all(void);

void sai_ipmc_group_dump(sai_object_id_t ipmc_group_obj);

void sai_ipmc_repl_group_dump_all(void);

void sai_ipmc_repl_group_dump(sai_object_id_t ipmc_repl_group_obj);

void sai_ipmc_group_member_dump_all(void);

void sai_ipmc_group_member_dump(sai_object_id_t ipmc_group_member_obj);

void sai_ipmc_rpf_group_member_dump(sai_object_id_t ipmc_rpf_group_member_obj);

void sai_ipmc_entry_func_log_init(void);

void sai_ipmc_rpf_group_func_log_init(void);

void sai_ipmc_repl_group_func_log_init(void);

void sai_l3_ipmc_entry_log (int loglevel, dn_sai_l3_mcast_entry_node_t *l3_mcast_entry_node,
                                            char *p_info_str);
#endif
