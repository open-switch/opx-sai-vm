/*
 * Copyright (c) 2018 Dell Inc.
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

/*
* @file sai_samplepacket_port.c
*
* @brief This file contains SAI Samplepacket for port API's.
*
*************************************************************************/
#include "saiswitch.h"
#include "saitypes.h"
#include "saistatus.h"

#include "sai_port_common.h"
#include "sai_port_utils.h"
#include "sai_samplepacket_defs.h"
#include "sai_samplepacket_api.h"
#include "sai_samplepacket_util.h"

#include "std_assert.h"
#include "std_radix.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#define SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE (sizeof(dn_sai_samplepacket_acl_info_key_t) * 8)

static std_rt_table *sai_acl_info_per_port = NULL;

std_rt_table* sai_acl_info_per_port_tree_get(void)
{
    return sai_acl_info_per_port;
}

sai_status_t sai_samplepacket_handle_per_port (sai_object_id_t port_id,
                                               const sai_attribute_t *attr,
                                               sai_samplepacket_direction_t samplepacket_direction)
{
    sai_status_t                       ret                 = SAI_STATUS_SUCCESS;
    sai_port_application_info_t       *port_node = NULL;

    SAI_SAMPLEPACKET_LOG_TRACE ("Samplepacket is enabled in port 0x%"PRIx64" for direction %d", port_id,
            samplepacket_direction);

    port_node = sai_port_application_info_create_and_get (port_id);
    if (port_node == NULL) {
        SAI_SAMPLEPACKET_LOG_ERR ("Port Info is NULL for port 0x%"PRIx64"", port_id);
        return SAI_STATUS_FAILURE;
    }

    if (attr->value.oid != SAI_NULL_OBJECT_ID) {
        if (port_node->sample_object[samplepacket_direction] != SAI_NULL_OBJECT_ID) {
            SAI_SAMPLEPACKET_LOG_ERR ("Samplepacket session already applied");
            return SAI_STATUS_OBJECT_IN_USE;
        }

        ret = sai_samplepacket_session_port_add (attr->value.oid,
                port_id, samplepacket_direction, SAI_SAMPLEPACKET_MODE_PORT_BASED);
        if (ret != SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR("Could not add port 0x%"PRIx64" to the samplepacket session "
                                     "0x%"PRIx64"",
                                      port_id, attr->value.oid);
            return SAI_STATUS_FAILURE;
        }

        port_node->sample_object[samplepacket_direction] = attr->value.oid;
    } else {
        ret = sai_samplepacket_session_port_remove (port_node->sample_object[samplepacket_direction],
                port_id, samplepacket_direction, SAI_SAMPLEPACKET_MODE_PORT_BASED);
        if (ret != SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR("Could not remove port 0x%"PRIx64" to the samplepacket session"
                                     "0x%"PRIx64"", port_id, attr->value.oid);
            return SAI_STATUS_FAILURE;
        }
        port_node->sample_object[samplepacket_direction] = SAI_NULL_OBJECT_ID;
        sai_port_application_info_remove(port_node);
    }

    return ret;
}

sai_status_t sai_acl_samplepacket_init ()
{
    sai_acl_info_per_port = std_radix_create ("sai_acl_info_per_port",
            SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE, NULL, NULL, 0);

    return SAI_STATUS_SUCCESS;
}

static void sai_port_list_get(sai_object_list_t *port_list)
{
    uint_t count = 0;
    STD_ASSERT(port_list != NULL);
    STD_ASSERT(port_list->list != NULL);
    sai_port_info_t *port_info = NULL;

    for (port_info = sai_port_info_getfirst(); (port_info != NULL);
            port_info = sai_port_info_getnext(port_info)) {
        port_list->list[count] = port_info->sai_port_id;
        count++;
    }

    port_list->count = count;
}


static sai_status_t sai_samplepacket_acl_is_same_sample_on_port (sai_object_id_t port_id,
                                                                 sai_object_id_t sample_object,
                                                                 sai_samplepacket_direction_t
                                                                                sample_direction)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_acl_info_t *p_acl_info = NULL;
    dn_sai_samplepacket_acl_info_key_t key;

    memset (&key, 0, sizeof(key));

    key.port_id = port_id;
    key.samplepacket_direction = sample_direction;

    p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getexact (
            sai_acl_info_per_port_tree_get(), (u_char *)&key,
            SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

    if (p_acl_info == NULL) {
        p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getnext (
                sai_acl_info_per_port_tree_get(), (u_char *)&key,
                SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

        /*
         * Check whether for the same port a different samplepacket session
         * is being applied other than the one already applied. if yes return failure.
         */
        while ((p_acl_info != NULL) &&
                (p_acl_info->key.port_id == port_id))
        {
            if ((p_acl_info->key.sample_object != sample_object) &&
                    (p_acl_info->key.samplepacket_direction == sample_direction)) {
                SAI_SAMPLEPACKET_LOG_ERR ("SamplePacket session cannot be overwritten "
                                          "for port 0x%"PRIx64" and direction %d",
                                          port_id, sample_direction);
                rc = SAI_STATUS_FAILURE;
                break;
            }

            p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getnext (
                    sai_acl_info_per_port_tree_get(), (u_char *)&p_acl_info->key,
                    SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);
        }

    }
    return rc;
}

static sai_status_t sai_samplepacket_check_and_insert_acl_info  (sai_object_id_t port_id,
                                                                 sai_object_id_t sample_object,
                                                                 sai_samplepacket_direction_t
                                                                               sample_direction)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_acl_info_t *p_acl_info = NULL;
    dn_sai_samplepacket_acl_info_key_t key;

    memset (&key, 0, sizeof(key));

    if (!(sai_acl_info_per_port_tree_get()))  {
        return SAI_STATUS_FAILURE;
    }

    key.port_id = port_id;
    key.sample_object = sample_object;
    key.samplepacket_direction = sample_direction;

    p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getexact (
                 sai_acl_info_per_port_tree_get(), (u_char *)&key,
                 SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

    if (p_acl_info == NULL) {
        p_acl_info = (dn_sai_samplepacket_acl_info_t *) calloc (1,
                                                        sizeof(dn_sai_samplepacket_acl_info_t));
        p_acl_info->key.sample_object = sample_object;
        p_acl_info->key.samplepacket_direction = sample_direction;
        p_acl_info->key.port_id = port_id;

        p_acl_info->head.rth_addr = (u_char *) &p_acl_info->key;

        p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_insert (
                sai_acl_info_per_port_tree_get(), &p_acl_info->head,
                SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);
        if (p_acl_info == NULL) {
            SAI_SAMPLEPACKET_LOG_ERR ("SamplePacket session insertion failed"
                                      "for port 0x%"PRIx64" and direction %d",
                                      port_id, sample_direction);
            free (p_acl_info);
            rc = SAI_STATUS_FAILURE;
        }

    }

    return rc;
}

static sai_status_t sai_samplepacket_validate_object_on_portlist  (sai_object_list_t *portlist,
                                                                   sai_object_id_t sample_object,
                                                                   sai_samplepacket_direction_t
                                                                   sample_direction)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_uint32_t port_count = 0;
    sai_object_id_t port_id = 0;

    STD_ASSERT (portlist != NULL);
    STD_ASSERT (portlist->list != NULL);

    for (port_count = 0; port_count < portlist->count; ++port_count)
    {
        port_id = portlist->list[port_count];
        rc = sai_samplepacket_acl_is_same_sample_on_port (port_id,
                sample_object,
                sample_direction);

        if (rc != SAI_STATUS_SUCCESS)
        {
            SAI_SAMPLEPACKET_LOG_ERR ("SamplePacket session could not be applied on the port"
                    "0x%"PRIx64" and direction %d",
                    port_id, sample_direction);
            return rc;
        }

    }

    return rc;
}

static sai_status_t sai_samplepacket_update_object_on_portlist  (sai_object_list_t *portlist,
                                                                 sai_object_id_t sample_object,
                                                                 sai_samplepacket_direction_t
                                                                   sample_direction)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_uint32_t port_count = 0;
    sai_object_id_t port_id = 0;

    STD_ASSERT (portlist != NULL);
    STD_ASSERT (portlist->list != NULL);

    for (port_count = 0; port_count < portlist->count; ++port_count)
    {
        port_id = portlist->list[port_count];
        rc = sai_samplepacket_session_port_add (sample_object,
                port_id, sample_direction, SAI_SAMPLEPACKET_MODE_FLOW_BASED);
        if (rc != SAI_STATUS_SUCCESS) {
            SAI_SAMPLEPACKET_LOG_ERR("Could not add port 0x%"PRIx64" to the samplepacket session 0x%"PRIx64"",
                                      port_id, sample_object);
            return rc;
        }


        rc = sai_samplepacket_check_and_insert_acl_info (port_id,
                sample_object,
                sample_direction);

        if (rc != SAI_STATUS_SUCCESS)
        {
            SAI_SAMPLEPACKET_LOG_ERR ("SamplePacket session 0x%"PRIx64" acl cache could not be "
                                      "updated for the port "
                                      "0x%"PRIx64" and direction %d",
                                       sample_object, port_id, sample_direction);
            sai_samplepacket_session_port_remove (sample_object,
                    port_id, sample_direction, SAI_SAMPLEPACKET_MODE_FLOW_BASED);
        }
    }

    return rc;
}

sai_status_t sai_samplepacket_validate_object  (sai_object_list_t *portlist,
                                                sai_object_id_t sample_object,
                                                sai_samplepacket_direction_t sample_direction,
                                                bool validate,
                                                bool update)
{
    sai_status_t rc = SAI_STATUS_SUCCESS;
    sai_object_list_t gen_port_list;

    memset (&gen_port_list, 0, sizeof(gen_port_list));

    if (!validate && !update) {
        SAI_SAMPLEPACKET_LOG_WARN ("No action to be performed");
        return SAI_STATUS_SUCCESS;
    }

    do {
        if (portlist == NULL) {
            gen_port_list.list = (sai_object_id_t *) calloc(sai_switch_get_max_lport(),
                    sizeof(sai_object_id_t));

            if(gen_port_list.list == NULL) {
                SAI_SAMPLEPACKET_LOG_ERR ("Allocation of Memory failed for port object list");
                rc = SAI_STATUS_NO_MEMORY;
                break;
            }

            sai_port_list_get (&gen_port_list);

            if (gen_port_list.count == 0) {
                SAI_SAMPLEPACKET_LOG_ERR ("Ports are not configured on the system");
                rc = SAI_STATUS_FAILURE;
                break;
            } else if (gen_port_list.count > sai_switch_get_max_lport()) {
                free (gen_port_list.list);
                gen_port_list.list = (sai_object_id_t *) calloc(gen_port_list.count,
                                     sizeof(sai_object_id_t));

                if(gen_port_list.list == NULL) {
                    SAI_SAMPLEPACKET_LOG_ERR ("Allocation of Memory failed for port object list");
                    rc = SAI_STATUS_NO_MEMORY;
                    break;
                }

                sai_port_list_get (&gen_port_list);
            }

            if (validate) {
                rc = sai_samplepacket_validate_object_on_portlist (&gen_port_list,
                        sample_object,
                        sample_direction);

                if (rc != SAI_STATUS_SUCCESS) {
                    break;
                }
            }

            if (update) {
                rc = sai_samplepacket_update_object_on_portlist (&gen_port_list,
                        sample_object,
                        sample_direction);

                if (rc != SAI_STATUS_SUCCESS) {
                   break;
                }

                /*
                 * A dummy node is inserted to identify that a samplepacket session is applied on all the
                 * ports to be used for dynamic fanout
                 */
                rc = sai_samplepacket_check_and_insert_acl_info (SAI_NULL_OBJECT_ID,
                        sample_object,
                        sample_direction);

                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_SAMPLEPACKET_LOG_WARN ("SamplePacket session 0x%"PRIx64" acl cache could not be"
                            "updated for dummy port and direction %d",
                            sample_object, sample_direction);
                }
            }

        } else {
            if (validate) {
                rc = sai_samplepacket_validate_object_on_portlist (portlist,
                        sample_object,
                        sample_direction);

                if (rc != SAI_STATUS_SUCCESS) {
                    return rc;
                }
            }

            if (update) {
                rc = sai_samplepacket_update_object_on_portlist (portlist,
                        sample_object,
                        sample_direction);

                if (rc != SAI_STATUS_SUCCESS) {
                    return rc;
                }
            }
        }
    } while (0);

    if (gen_port_list.list != NULL) {
        free (gen_port_list.list);
    }

    return rc;
}

sai_status_t sai_samplepacket_remove_object (sai_object_list_t *portlist,
                                            sai_object_id_t sample_object,
                                            sai_samplepacket_direction_t sample_direction)

{
    sai_uint32_t port_count = 0;
    sai_status_t rc = SAI_STATUS_SUCCESS;
    dn_sai_samplepacket_acl_info_t *p_acl_info = NULL;
    dn_sai_samplepacket_acl_info_key_t key;
    sai_port_info_t *port_info = NULL;

    memset (&key, 0, sizeof(key));

    if (!(sai_acl_info_per_port_tree_get()))  {
        return SAI_STATUS_FAILURE;
    }

    key.sample_object = sample_object;
    key.samplepacket_direction = sample_direction;

    if (portlist == NULL) {

        /*
         * Remove the dummy node
         */
        key.port_id = SAI_NULL_OBJECT_ID;
        p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getexact (
                sai_acl_info_per_port_tree_get(), (u_char *)&key,
                SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

        if (p_acl_info) {
            std_radix_remove (sai_acl_info_per_port_tree_get(), &p_acl_info->head);
            free (p_acl_info);
            p_acl_info = NULL;
        }

        for (port_info = sai_port_info_getfirst(); (port_info != NULL);
                port_info = sai_port_info_getnext(port_info)) {

            key.port_id = port_info->sai_port_id;
            p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getexact (
                         sai_acl_info_per_port_tree_get(), (u_char *)&key,
                         SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

            if (p_acl_info) {
                rc = sai_samplepacket_session_port_remove (sample_object,
                        port_info->sai_port_id, sample_direction, SAI_SAMPLEPACKET_MODE_FLOW_BASED);
                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_SAMPLEPACKET_LOG_WARN("Could not remove port 0x%"PRIx64" to the samplepacket "
                                              "session 0x%"PRIx64"",
                                               port_info->sai_port_id, sample_object);
                } else {
                    std_radix_remove (sai_acl_info_per_port_tree_get(), &p_acl_info->head);
                    free (p_acl_info);
                }
            }
        }
    } else {
        for (port_count = 0; port_count < portlist->count; ++port_count)
        {
            key.port_id = portlist->list[port_count];
            p_acl_info = (dn_sai_samplepacket_acl_info_t *) std_radix_getexact (
                    sai_acl_info_per_port_tree_get(), (u_char *)&key,
                    SAI_ACL_INFO_PER_PORT_TREE_KEY_SIZE);

            if (p_acl_info) {
                rc = sai_samplepacket_session_port_remove (sample_object,
                        portlist->list[port_count], sample_direction, SAI_SAMPLEPACKET_MODE_FLOW_BASED);
                if (rc != SAI_STATUS_SUCCESS) {
                    SAI_SAMPLEPACKET_LOG_WARN ("Could not remove port 0x%"PRIx64" to the samplepacket "
                                               "session 0x%"PRIx64"",
                                               portlist->list[port_count], sample_object);
                } else {
                    std_radix_remove (sai_acl_info_per_port_tree_get(), &p_acl_info->head);
                    free (p_acl_info);
                }
            }
        }
    }

    return rc;
}
