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
 * \file   sai_shell_debug_handler.c
 * \brief  SAI module debug handlers
 */

#include <stdio.h>
#include "sai_vlan_main.h"
#include "sai_stp_debug.h"
#include "sai_shell.h"
#include "std_utils.h"
#include "sai_debug_utils.h"
#include "sai_common_acl.h"
#include "string.h"
#include "sai_fdb_main.h"
#include "sai_vlan_common.h"
#include "sai_common_infra.h"
#include "sai_debug_utils.h"
#include "sai_lag_debug.h"
#include "sai_mirror_api.h"
#include "sai_samplepacket_api.h"
#include "sai_qos_debug.h"
#include "sai_l3_api_utils.h"
#include "sai_bridge_main.h"
#include "sai_l2mc_api.h"

static void sai_shell_debug_vlan_help(void)
{
    SAI_DEBUG("::debug vlan global <vlan-id>/all");
    SAI_DEBUG("\t- Dumps the VLAN global info");
    SAI_DEBUG("::debug vlan member [<vlan-id> <sai-port-id>]/all");
    SAI_DEBUG("\t- Dumps the VLAN member tree");

}
static void sai_shell_debug_fdb_help(void)
{
    SAI_DEBUG("::debug fdb global all");
    SAI_DEBUG("\t- Dumps all the learnt FDB entries");
    SAI_DEBUG("::debug fdb global count");
    SAI_DEBUG("\t- Dumps the count of FDB entries");
    SAI_DEBUG("::debug fdb param <sai-port> <vlan-id> ");
    SAI_DEBUG("\t- Dumps all the learnt FDB entry per port/vlan/port-vlan.");
    SAI_DEBUG("::debug fdb registered all");
    SAI_DEBUG("\t- Dumps the FDB nodes registerd for callback by L3 Neighbor entries");
    SAI_DEBUG("::debug fdb registered pending");
    SAI_DEBUG("\t- Dumps the pending FDB nodes registerd for callback by L3 Neighbor entries");
}

static void sai_shell_debug_l3_help(void)
{
    SAI_DEBUG("::debug l3 vr");
    SAI_DEBUG("\t- Debug commands related to Virtual Router");
    SAI_DEBUG("::debug l3 rif");
    SAI_DEBUG("\t- Debug commands related to Router Interface");
    SAI_DEBUG("::debug l3 neighbor");
    SAI_DEBUG("\t- Debug commands related to Neighbor");
    SAI_DEBUG("::debug l3 nexthop");
    SAI_DEBUG("\t- Debug commands related to Nexthop");
    SAI_DEBUG("::debug l3 nexthop-group");
    SAI_DEBUG("\t- Debug commands related to Nexthop group");
    SAI_DEBUG("::debug l3 route");
    SAI_DEBUG("\t- Debug commands related to Route");
}

static void sai_shell_debug_qos_help(void)
{
    SAI_DEBUG("::debug qos port");
    SAI_DEBUG("\t- Debug commands related to QOS port related information");
    SAI_DEBUG("::debug qos scheduler-group");
    SAI_DEBUG("\t- Debug commands related to Scheduler-group");
    SAI_DEBUG("::debug qos scheduler");
    SAI_DEBUG("\t- Debug commands related to Sceduler");
    SAI_DEBUG("::debug qos queue");
    SAI_DEBUG("\t- Debug commands related to Queue");
    SAI_DEBUG("::debug qos ingress-pg");
    SAI_DEBUG("\t- Debug commands related to Ingress priority group");
    SAI_DEBUG("::debug qos buffer-pool");
    SAI_DEBUG("\t- Debug commands related to Buffer pool");
    SAI_DEBUG("::debug qos buffer-profile");
    SAI_DEBUG("\t- Debug commands related to Buffer profile");
    SAI_DEBUG("::debug qos maps");
    SAI_DEBUG("\t- Debug commands related to Qos Maps");
    SAI_DEBUG("::debug qos wred");
    SAI_DEBUG("\t- Debug commands related to Wred");
    SAI_DEBUG("::debug qos policer");
    SAI_DEBUG("\t- Debug commands related to Policer");
}

static void sai_shell_debug_vr_help(void)
{
    SAI_DEBUG("::debug l3 vr all");
    SAI_DEBUG("\t- Dumps all the virtual router entries");
    SAI_DEBUG("::debug l3 vr <vr_id> ");
    SAI_DEBUG("\t- Dumps the virtual router with object id vr_id.");
}

static void sai_shell_debug_rif_help(void)
{
    SAI_DEBUG("::debug l3 rif all");
    SAI_DEBUG("\t- Dumps all the router interface data");
    SAI_DEBUG("::debug l3 rif vr <vr_id> ");
    SAI_DEBUG("\t- Dumps all the router interface data in virtual router vr_id.");
    SAI_DEBUG("::debug l3 rif <rif_id> ");
    SAI_DEBUG("\t- Dumps the router interface data with object id rif_id.");
}

static void sai_shell_debug_neighbor_help(void)
{
    SAI_DEBUG("::debug l3 neighbor vr <vr_id> ");
    SAI_DEBUG("\t- Dumps all the neighbor data in virtual router vr_id.");
    SAI_DEBUG("::debug l3 neighbor mac-tree ");
    SAI_DEBUG("\t- Dumps the neighbor MAC entry tree");
}

static void sai_shell_debug_nexthop_help(void)
{
    SAI_DEBUG("::debug l3 nexthop all");
    SAI_DEBUG("\t- Dumps all the nexthop data");
    SAI_DEBUG("::debug l3 nexthop <nh_id> ");
    SAI_DEBUG("\t- Dumps the nexthop data with object id <nh_id>.");
}

static void sai_shell_debug_nexthop_group_help(void)
{
    SAI_DEBUG("::debug l3 nexthop-group all");
    SAI_DEBUG("\t- Dumps all the nexthop-group data");
    SAI_DEBUG("::debug l3 nexthop-group <nhg_id> ");
    SAI_DEBUG("\t- Dumps the nexthop-group data with object id <nhg_id>.");
}

static void sai_shell_debug_route_help(void)
{
    SAI_DEBUG("::debug l3 router vr <vr_id> ");
    SAI_DEBUG("\t- Dumps all the route entry data in virtual router vr_id.");
}

static void sai_shell_debug_lag_help(void)
{
    SAI_DEBUG("::debug lag <sai-lag-id>/<trunk-id>/all ");
    SAI_DEBUG("\t- Dumps the LAG sai info");
}

static void sai_shell_debug_mirror_help(void)
{
    SAI_DEBUG("::debug mirror <mirror-session-id>/all ");
    SAI_DEBUG("\t- Dumps the MIRROR session info");
}

static void sai_shell_debug_sflow_help(void)
{
    SAI_DEBUG("::debug sflow <mirror-session-id>/all ");
    SAI_DEBUG("\t- Dumps the SFLOW (sample packet) session info");
}

static void sai_shell_debug_qos_port_help(void)
{
    SAI_DEBUG("::debug qos port <port-id>");
    SAI_DEBUG("\t- Dumps all QOS configs applied on the port");
    SAI_DEBUG("::debug qos port hierarchy <port-id> ");
    SAI_DEBUG("\t- Dumps QOS port hierarchy on the port.");
}

static void sai_shell_debug_queue_help(void)
{
    SAI_DEBUG("::debug qos queue <queue-id>");
    SAI_DEBUG("\t- Dumps the queue info");
    SAI_DEBUG("::debug qos queue all ");
    SAI_DEBUG("\t- Dumps all the queue info");
    SAI_DEBUG("::debug qos queue parent <queue-id>");
    SAI_DEBUG("\t- Dumps all the parents of queue specified");
}

static void sai_shell_debug_sg_help(void)
{
    SAI_DEBUG("::debug qos scheduler-group <sg-id>");
    SAI_DEBUG("\t- Dumps the sceduler-group info");
    SAI_DEBUG("::debug qos scheduler-group all ");
    SAI_DEBUG("\t- Dumps all the scheduler-group info");
    SAI_DEBUG("::debug qos scheduler-group parent <sg-id>");
    SAI_DEBUG("\t- Dumps all the parents of sceduler-group specified");
}

static void sai_shell_debug_scheduler_help(void)
{
    SAI_DEBUG("::debug qos scheduler <scheduler-id>");
    SAI_DEBUG("\t- Dumps the sceduler info");
    SAI_DEBUG("::debug qos scheduler all ");
    SAI_DEBUG("\t- Dumps all the scheduler info");
}

static void sai_shell_debug_pg_help(void)
{
    SAI_DEBUG("::debug qos ingress-pg <pg-id>");
    SAI_DEBUG("\t- Dumps the ingress priority-group info");
    SAI_DEBUG("::debug qos ingress-pg all ");
    SAI_DEBUG("\t- Dumps all the ingress priority-group info");
}

static void sai_shell_debug_buffer_profile_help(void)
{
    SAI_DEBUG("::debug qos buffer-profile <profile-id>");
    SAI_DEBUG("\t- Dumps the buffer-profile info");
    SAI_DEBUG("::debug qos buffer-profile all ");
    SAI_DEBUG("\t- Dumps all the buffer-profile info");
}

static void sai_shell_debug_buffer_pool_help(void)
{
    SAI_DEBUG("::debug qos buffer-pool <pool-id>");
    SAI_DEBUG("\t- Dumps the buffer-pool info");
    SAI_DEBUG("::debug qos buffer-pool all ");
    SAI_DEBUG("\t- Dumps all the buffer-pool info");
}

static void sai_shell_debug_qos_maps_help(void)
{
    SAI_DEBUG("::debug qos maps <maps-id>");
    SAI_DEBUG("\t- Dumps the maps info");
    SAI_DEBUG("::debug qos maps all ");
    SAI_DEBUG("\t- Dumps all the maps info");
}

static void sai_shell_debug_wred_help(void)
{
    SAI_DEBUG("::debug qos wred <wred-id>");
    SAI_DEBUG("\t- Dumps the wred info");
    SAI_DEBUG("::debug qos wred all ");
    SAI_DEBUG("\t- Dumps all the wred info");
}

static void sai_shell_debug_policer_help(void)
{
    SAI_DEBUG("::debug qos policer <policer-id>");
    SAI_DEBUG("\t- Dumps the policer info");
    SAI_DEBUG("::debug qos policer all ");
    SAI_DEBUG("\t- Dumps all the policer info");
}

static void sai_shell_debug_bridge_help(void)
{
    SAI_DEBUG("::debug bridge all");
    SAI_DEBUG("\t- Dumps all the bridge info");
    SAI_DEBUG("::debug bridge ids");
    SAI_DEBUG("\t- Dumps all the bridge ids");
    SAI_DEBUG("::debug bridge default");
    SAI_DEBUG("\t- Dumps all the default 1Q bridge");
    SAI_DEBUG("::debug bridge info <bridge-id>");
    SAI_DEBUG("\t- Dumps the bridge info");
    SAI_DEBUG("::debug bridge port all");
    SAI_DEBUG("\t- Dumps all the bridge port info");
    SAI_DEBUG("::debug bridge port ids");
    SAI_DEBUG("\t- Dumps all the bridge port ids");
    SAI_DEBUG("::debug bridge port info <bridge-port-id>");
    SAI_DEBUG("\t- Dumps the bridge port info");

}

static void sai_shell_debug_l2mc_help(void)
{
    SAI_DEBUG("::debug l2mc group all");
    SAI_DEBUG("\t- Dumps all the l2mc group info");
    SAI_DEBUG("::debug l2mc group <group-id>");
    SAI_DEBUG("\t- Dumps the specific l2mc group info");
    SAI_DEBUG("::debug l2mc member all");
    SAI_DEBUG("\t- Dumps all the l2mc group member info");
    SAI_DEBUG("::debug l2mc member <member-id>");
    SAI_DEBUG("\t- Dumps the specific l2mc group member info");
}

static void sai_shell_debug_vlan(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_vlan_id_t vlan_id;
    sai_object_id_t port_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")){
            sai_shell_debug_vlan_help();
        } else if(!strcmp(token,"global")) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(!strcmp(token,"all")){
                    sai_vlan_dump_global_info(0,true);
                } else {
                    sscanf(token,"%hu",&vlan_id);
                    sai_vlan_dump_global_info(vlan_id,false);
                }
            }
        } else if(!strcmp(token,"member")) {
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_vlan_dump_member_info(0,0,true);
                } else {
                    sscanf(token,"%hu",&vlan_id);

                    if((token = std_parse_string_next(handle,&ix)) != NULL) {
                        port_id = strtol(token,NULL,0);
                        sai_vlan_dump_member_info(vlan_id,port_id,false);
                    }
                }
            }
        }
    } else {
        sai_shell_debug_vlan_help();
    }
}

static void sai_shell_debug_fdb(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_vlan_id_t vlan_id = VLAN_UNDEF;
    sai_object_id_t bridge_port_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_fdb_help();
        } else if(strcmp(token,"global") == 0) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(strcmp(token,"all") == 0) {
                    sai_dump_all_fdb_entry_nodes();
                } else if (strcmp(token,"count") == 0) {
                    sai_dump_all_fdb_entry_count();
                } else {
                    SAI_DEBUG ("Invalid parameters");
                }
            } else {
                SAI_DEBUG("Missing parameter");
            }
        } else if(strcmp(token,"param") == 0) {
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                sscanf(token,"%lx",&bridge_port_id);
                if(std_parse_string_num_tokens(handle) == 4) {
                    sscanf(std_parse_string_next(handle,&ix),"%hu",&vlan_id);
                }
                if(bridge_port_id == SAI_NULL_OBJECT_ID) {
                    if (vlan_id != VLAN_UNDEF) {
                        sai_dump_fdb_entry_nodes_per_vlan (vlan_id);
                    } else {
                        SAI_DEBUG ("Invalid parameters");
                    }
                } else {
                    if(vlan_id != VLAN_UNDEF) {
                        sai_dump_fdb_entry_nodes_per_bridge_port_vlan(bridge_port_id, vlan_id);
                    } else {
                        sai_dump_fdb_entry_nodes_per_bridge_port(bridge_port_id);
                    }
                }
            }
        } else if(strcmp(token,"registered") == 0) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(strcmp(token,"all") == 0) {
                    sai_dump_all_fdb_registered_nodes();
                } else {
                    SAI_DEBUG ("Invalid parameters");
                }
            }
        } else {
            SAI_DEBUG ("Unknown parameter");
        }
    } else {
        sai_shell_debug_fdb_help();
    }
}

static void sai_shell_debug_vr(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_vr_help();
        } else if(strcmp(token,"all") == 0) {
            sai_fib_dump_all_vr ();
        } else {
            sscanf(token,"%lx",&vr_id);
            if(vr_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_vr(vr_id);
            }
        }
    } else {
        sai_shell_debug_vr_help();
    }
}

static void sai_shell_debug_rif(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t  rif_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_rif_help();
        } else if(strcmp(token,"all") == 0) {
            sai_fib_dump_all_rif ();
        } else if(strcmp(token,"vr") == 0){
            sscanf(std_parse_string_next(handle,&ix),"%lx",&vr_id);
            if(vr_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_all_rif_in_vr(vr_id);
            }
        } else {
            sscanf(token,"%lx",&rif_id);
            if(rif_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_rif (rif_id);
            }
        }
    } else {
        sai_shell_debug_rif_help();
    }
}

static void sai_shell_debug_neighbor(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_neighbor_help();
        } else if(strcmp(token,"vr") == 0){
            sscanf(std_parse_string_next(handle,&ix),"%lx",&vr_id);
            if(vr_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_all_neighbor_in_vr (vr_id);
            }
        } else if(strcmp(token,"mac-tree") == 0){
            sai_fib_dump_neighbor_mac_entry_tree ();
        }
    } else {
        sai_shell_debug_neighbor_help();
    }
}

static void sai_shell_debug_nexthop(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  nh_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_nexthop_help();
        } else if (strcmp(token,"all") == 0) {
            sai_fib_dump_all_nh ();
        } else {
            sscanf(token,"%lx",&nh_id);
            if(nh_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_nh (nh_id);
            }
        }
    } else {
        sai_shell_debug_nexthop_help();
    }
}

static void sai_shell_debug_nexthop_group(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  nhg_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_nexthop_group_help();
        } else if (strcmp(token,"all") == 0) {
            sai_fib_dump_all_nh_group ();
        }  else {
            sscanf(token,"%lx",&nhg_id);
            if(nhg_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_nh_group (nhg_id);
            }
        }
    } else {
        sai_shell_debug_nexthop_group_help();
    }
}

static void sai_shell_debug_route(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  vr_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_route_help();
        } else if(strcmp(token,"vr") == 0){
            sscanf(std_parse_string_next(handle,&ix),"%lx",&vr_id);
            if(vr_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameters");
            } else {
                sai_fib_dump_all_route_in_vr (vr_id);
            }
        } else {
            SAI_DEBUG ("Invalid parameter");
        }
    } else {
        sai_shell_debug_route_help();
    }
}

static void sai_shell_debug_l3(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_l3_help();
        } else if(strcmp(token,"vr") == 0) {
            sai_shell_debug_vr(handle);
        } else if(strcmp(token,"rif") == 0) {
            sai_shell_debug_rif(handle);
        } else if(strcmp(token,"neighbor") == 0) {
            sai_shell_debug_neighbor(handle);
        } else if(strcmp(token,"nexthop-group") == 0) {
            sai_shell_debug_nexthop_group(handle);
        } else if(strcmp(token,"nexthop") == 0) {
            sai_shell_debug_nexthop(handle);
        } else if(strcmp(token,"route") == 0) {
            sai_shell_debug_route(handle);
        } else {
            SAI_DEBUG ("Unknown parameter");
        }
    } else {
        sai_shell_debug_l3_help();
    }
    return;
}

static void sai_shell_debug_qos_port(std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  port_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_qos_port_help();
        } else if(strcmp(token,"hierarchy") == 0) {
            token = std_parse_string_next(handle,&ix);
            if(token != NULL) {
                sscanf(token,"%lx",&port_id);
                sai_dump_port_hierarchy (port_id);
            } else {
                SAI_DEBUG ("Invalid parameter");
            }
        } else {
            sscanf(token,"%lx",&port_id);
            if(port_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_qos_port_info (port_id);
            }
        }
    } else {
        sai_shell_debug_qos_port_help();
    }
}

static void sai_shell_debug_sg (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  sg_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_sg_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_sg_nodes ();
        } else if (strcmp(token,"parent") == 0) {
            sscanf(token,"%lx",&sg_id);
            if(sg_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_all_parents (sg_id);
            }
        } else {
            sscanf(token,"%lx",&sg_id);
            if(sg_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_sg_info (sg_id);
            }
        }
    } else {
        sai_shell_debug_sg_help();
    }
}

static void sai_shell_debug_queue (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  queue_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_queue_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_queue_nodes ();
        } else if (strcmp(token,"parent") == 0) {
            sscanf(token,"%lx",&queue_id);
            if(queue_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_all_parents (queue_id);
            }
        } else {
            sscanf(token,"%lx",&queue_id);
            if(queue_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_queue_info (queue_id);
            }
        }
    } else {
        sai_shell_debug_queue_help();
    }
}

static void sai_shell_debug_scheduler (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  sched_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_scheduler_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_scheduler_nodes ();
        } else {
            sscanf(token,"%lx",&sched_id);
            if(sched_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_scheduler_info (sched_id);
            }
        }
    } else {
        sai_shell_debug_scheduler_help();
    }
}

static void sai_shell_debug_pg (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  pg_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_pg_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_pg_nodes ();
        } else {
            sscanf(token,"%lx",&pg_id);
            if(pg_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_pg_info (pg_id);
            }
        }
    } else {
        sai_shell_debug_pg_help();
    }
}

static void sai_shell_debug_buffer_profile (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  buffer_profile_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_buffer_profile_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_buffer_profile_nodes ();
        } else {
            sscanf(token,"%lx",&buffer_profile_id);
            if(buffer_profile_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_buffer_profile_info (buffer_profile_id);
            }
        }
    } else {
        sai_shell_debug_buffer_profile_help();
    }
}

static void sai_shell_debug_buffer_pool (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  buffer_pool_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_buffer_pool_help();
        } else if(strcmp(token,"all") == 0) {
            sai_dump_all_buffer_pool_nodes ();
        } else {
            sscanf(token,"%lx",&buffer_pool_id);
            if(buffer_pool_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_dump_buffer_pool_info (buffer_pool_id);
            }
        }
    } else {
        sai_shell_debug_buffer_pool_help();
    }
}

static void sai_shell_debug_qos_maps (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  maps_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_qos_maps_help();
        } else if(strcmp(token,"all") == 0) {
            sai_qos_maps_dump_all ();
        } else {
            sscanf(token,"%lx",&maps_id);
            if(maps_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_qos_maps_dump (maps_id);
            }
        }
    } else {
        sai_shell_debug_qos_maps_help();
    }
}

static void sai_shell_debug_wred (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  wred_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_wred_help();
        } else if(strcmp(token,"all") == 0) {
            sai_qos_wred_dump_all ();
        } else {
            sscanf(token,"%lx",&wred_id);
            if(wred_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_qos_wred_dump (wred_id);
            }
        }
    } else {
        sai_shell_debug_wred_help();
    }
}

static void sai_shell_debug_policer (std_parsed_string_t handle)
{
    size_t ix=2;
    const char *token = NULL;
    sai_object_id_t  policer_id = SAI_NULL_OBJECT_ID;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_policer_help();
        } else if(strcmp(token,"all") == 0) {
            sai_qos_policer_dump_all ();
        } else {
            sscanf(token,"%lx",&policer_id);
            if(policer_id == SAI_NULL_OBJECT_ID) {
                SAI_DEBUG ("Invalid parameter");
            } else {
                sai_qos_policer_dump (policer_id);
            }
        }
    } else {
        sai_shell_debug_policer_help();
    }
}

static void sai_shell_debug_qos(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0) {
            sai_shell_debug_qos_help();
        } else if(strcmp(token,"port") == 0) {
            sai_shell_debug_qos_port(handle);
        } else if(strcmp(token,"scheduler-group") == 0) {
            sai_shell_debug_sg(handle);
        } else if(strcmp(token,"scheduler") == 0) {
            sai_shell_debug_scheduler(handle);
        } else if(strcmp(token,"queue") == 0) {
            sai_shell_debug_queue(handle);
        } else if(strcmp(token,"ingress-pg") == 0) {
            sai_shell_debug_pg(handle);
        } else if(strcmp(token,"buffer-pool") == 0) {
            sai_shell_debug_buffer_pool(handle);
        } else if(strcmp(token,"buffer-profile") == 0) {
            sai_shell_debug_buffer_profile(handle);
        } else if(strcmp(token,"maps") == 0) {
            sai_shell_debug_qos_maps(handle);
        } else if(strcmp(token,"wred") == 0) {
            sai_shell_debug_wred(handle);
        } else if(strcmp(token,"policer") == 0) {
            sai_shell_debug_policer(handle);
        } else {
            SAI_DEBUG ("Unknown parameter");
            sai_shell_debug_qos_help();
        }
    } else {
        sai_shell_debug_qos_help();
    }
    return;
}
static void sai_shell_debug_stp_help(void)
{
    SAI_DEBUG("::debug stp inst <sai-stp-id>/<stg-id>/all");
    SAI_DEBUG("\t- Dumps the STP global info");

    return;
}

static void sai_shell_debug_stp(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t stp_inst_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_stp_help();
        } else if(!strcmp(token,"inst")) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_stp_dump_global_info(0,true);
                } else {
                    stp_inst_id = strtol(token,NULL,0);
                    sai_stp_dump_global_info(stp_inst_id,false);
                }
            }
        }
    } else {
        sai_shell_debug_stp_help();
    }

    return;
}

static void sai_shell_debug_port_help(void)
{
    SAI_DEBUG("::debug port sai attr <sai_port_id>");
    SAI_DEBUG("::debug port sai info <sai_port_id>");
    SAI_DEBUG("::debug port sai dump all");
    SAI_DEBUG("::debug port bcm info <sai_port_id> or <all>");
    SAI_DEBUG("::debug port bcm hwinfo <sai_port_id> or <all>");
    SAI_DEBUG("::debug port bcm phyinfo <sai_port_id> or <all>");
    SAI_DEBUG("::debug port bcm default <sai_port_id> or <all>");
    SAI_DEBUG("::debug port bcm dump all");
    SAI_DEBUG("::debug port bcm mapinfo <sai_port_id>");

    return;
}

static void sai_shell_debug_acl_help(void)
{
    SAI_DEBUG("::debug acl table <tableid>/all");
    SAI_DEBUG("\t- Dumps the tableids ");
    SAI_DEBUG("::debug acl rule <ruleid>");
    SAI_DEBUG("\t- Dumps the ruleids ");
    SAI_DEBUG("::debug acl counter <eid>/all");
    SAI_DEBUG("\t- Dumps the hardware counters ");
    return;
}

static void sai_shell_debug_sai_port(std_parsed_string_t handle, size_t *ix)
{
    const char *token = NULL;
    sai_object_id_t port_id = 0;

    if((token = std_parse_string_next(handle,ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_port_help();
        }else if(!(strcmp(token,"attr"))) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                sscanf(token,"%lx",&port_id);
                sai_port_attr_info_dump_port(port_id);
            }
        } else if(!(strcmp(token,"info"))){
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                sscanf(token,"%lx",&port_id);
                sai_port_info_dump_port(port_id);
            }
        } else if(!(strcmp(token,"dump"))){
            sai_port_info_dump_all();
        } else {
            sai_shell_debug_port_help();
        }
    }
    if (NULL == token) {
        sai_shell_debug_port_help();
    }
}

static void sai_shell_debug_bcm_port(std_parsed_string_t handle, size_t *ix)
{
    const char *token = NULL;
    sai_object_id_t port_id = 0;

    if((token = std_parse_string_next(handle,ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_port_help();
        } else if(!(strcmp(token,"info"))) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_port_npu_api_get()->port_debug(SAI_BCM_PORT_DEBUG_DUMP_ALL,SAI_PORT_BCM_INFO);
                } else {
                    sscanf(token,"%lx",&port_id);
                    sai_port_npu_api_get()->port_debug(port_id,SAI_PORT_BCM_INFO);
                }
            }
        } else if(!strcmp(token,"hwinfo")) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_port_npu_api_get()->port_debug(SAI_BCM_PORT_DEBUG_DUMP_ALL,SAI_PORT_BCM_HWINFO);
                } else {
                    sscanf(token,"%lx",&port_id);
                    sai_port_npu_api_get()->port_debug(port_id,SAI_PORT_BCM_HWINFO);
                }
            }
        } else if(!strcmp(token,"phyinfo")) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_port_npu_api_get()->port_debug(SAI_BCM_PORT_DEBUG_DUMP_ALL,SAI_PORT_BCM_PHYINFO);
                } else {
                    sscanf(token,"%lx",&port_id);
                    sai_port_npu_api_get()->port_debug(port_id,SAI_PORT_BCM_PHYINFO);
                }
            }
        } else if(!strcmp(token,"default")) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_port_npu_api_get()->port_debug(SAI_BCM_PORT_DEBUG_DUMP_ALL,SAI_PORT_BCM_DEFAULT);
                } else {
                    sscanf(token,"%lx",&port_id);
                    sai_port_npu_api_get()->port_debug(port_id,SAI_PORT_BCM_DEFAULT);
                }
            }
        } else if(!strcmp(token,"dump")) {
            sai_port_npu_api_get()->port_debug(SAI_BCM_PORT_DEBUG_DUMP_ALL,SAI_PORT_BCM_DUMP_ALL);
        } else if(!strcmp(token,"mapinfo")) {
            token = std_parse_string_next(handle,ix);
            if(NULL != token) {
                sscanf(token,"%lx",&port_id);
                sai_port_npu_api_get()->port_debug(port_id,SAI_PORT_BCM_PORT_MAP);
            }
        } else {
            sai_shell_debug_port_help();
        }
    }
    if(NULL == token) {
        sai_shell_debug_port_help();
    }
}

static void sai_shell_debug_port(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;

    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }
    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_port_help();
        } else if(!strcmp(token,"sai")) {
            sai_shell_debug_sai_port(handle,&ix);
        } else if(!strcmp(token,"bcm")) {
            sai_shell_debug_bcm_port(handle,&ix);
        } else {
            sai_shell_debug_port_help();
        }
    } else {
        sai_shell_debug_port_help();
    }
}

static void sai_shell_debug_acl(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    int eid = 0;
    sai_object_id_t acl_table_id = SAI_NULL_OBJECT_ID;
    sai_object_id_t acl_rule_id = SAI_NULL_OBJECT_ID;


    if((std_parse_string_num_tokens(handle)) == 0) {
        return;
    }

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_acl_help();
        }
        else if(!strcmp(token,"counter")) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_acl_dump_counters();
                } else {
                    sscanf(token,"%d",&eid);
                    sai_acl_dump_counter_per_entry(eid);
                }
            }
        }
        else if(!strcmp(token,"table")) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                if(!(strcmp(token,"all"))) {
                    sai_acl_dump_all_tables();
                } else {
                    sscanf(token,"%lx",&acl_table_id);
                    sai_acl_dump_table(acl_table_id);
                }
            }
        }
        else if(!strcmp(token,"rule")) {
            token = std_parse_string_next(handle,&ix);

            if(NULL != token) {
                sscanf(token,"%lx",&acl_rule_id);
                sai_acl_dump_rule(acl_rule_id);
            }
        }
    }

    return;
}

static void sai_shell_debug_lag(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t lag_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_lag_help();
        } else {
            if(!(strcmp(token,"all"))) {
                sai_dump_lag_info(0,true);
            } else {
                lag_id = strtol(token,NULL,0);
                sai_dump_lag_info(lag_id,false);
            }
        }
    } else {
        sai_shell_debug_lag_help();
    }

    return;
}

static void sai_shell_debug_sflow(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t session_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_sflow_help();
        } else {
            if(!(strcmp(token,"all"))) {
                sai_samplepacket_dump(0,true);
            } else {
                session_id = strtol(token,NULL,0);
                sai_samplepacket_dump(session_id,false);
            }
        }
    } else {
        sai_shell_debug_sflow_help();
    }

    return;
}

static void sai_shell_debug_mirror(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t session_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_mirror_help();
        } else {
            if(!(strcmp(token,"all"))) {
                sai_mirror_dump(0,true);
            } else {
                session_id = strtol(token,NULL,0);
                sai_mirror_dump(session_id,false);
            }
        }
    } else {
        sai_shell_debug_mirror_help();
    }

    return;
}

static void sai_shell_debug_bridge(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t bridge_id;
    sai_object_id_t bridge_port_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0){
            sai_shell_debug_bridge_help();
        } else if(strcmp(token,"all") == 0) {
            sai_bridge_dump_all_bridge_info();
        } else if(strcmp(token, "ids") == 0){
            sai_bridge_dump_all_bridge_ids();
        } else if(strcmp(token,"default") == 0) {
            sai_brige_dump_default_bridge();
        } else if(strcmp(token,"info") == 0) {
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                sscanf(token,"%lx",&bridge_id);
                sai_bridge_dump_bridge_info(bridge_id);
            } else {
                SAI_DEBUG("Error - missing bridge id");
            }
        } else if(strcmp(token,"port") == 0) {
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                if(strcmp(token,"all") == 0) {
                    sai_bridge_dump_all_bridge_port_info();
                }else if(strcmp(token, "ids") == 0){
                    sai_bridge_dump_all_bridge_port_ids();
                } else if( strcmp(token,"info") == 0) {
                    token = std_parse_string_next(handle,&ix);
                    if(NULL != token) {
                        sscanf(token,"%lx",&bridge_port_id);
                        sai_bridge_dump_bridge_port_info(bridge_port_id);
                    } else {
                        SAI_DEBUG("Error - missing bridge port id");
                    }
                } else {
                    sai_shell_debug_bridge_help();
                }
            } else {
                sai_shell_debug_bridge_help();
            }
        } else {
            sai_shell_debug_bridge_help();
        }
    } else {
        sai_shell_debug_bridge_help();
    }
}

static void sai_shell_debug_l2mc(std_parsed_string_t handle)
{
    size_t ix=1;
    const char *token = NULL;
    sai_object_id_t l2mc_group_id;
    sai_object_id_t l2mc_group_member_id;

    if((token = std_parse_string_next(handle,&ix))!= NULL) {
        if(strcmp(token,"help") == 0){
            sai_shell_debug_l2mc_help();
        } else if(strcmp(token,"group") == 0) {
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                if(strcmp(token,"all") == 0) {
                    sai_l2mc_group_dump_all();
                } else {
                    sscanf(token,"%lx",&l2mc_group_id);
                    sai_l2mc_group_dump(l2mc_group_id);
                }
            } else {
                sai_shell_debug_l2mc_help();
            }
        } else if(strcmp(token, "member") == 0){
            token = std_parse_string_next(handle,&ix);
            if(NULL != token) {
                if(strcmp(token,"all") == 0) {
                    sai_l2mc_member_dump_all();
                } else {
                    sscanf(token,"%lx",&l2mc_group_member_id);
                    sai_l2mc_member_dump(l2mc_group_member_id);
                }
            } else {
                sai_shell_debug_l2mc_help();
            }
        } else {
            sai_shell_debug_l2mc_help();
        }
    } else {
        sai_shell_debug_l2mc_help();
    }
}
static void sai_shell_debug_help(void)
{
    SAI_DEBUG("::debug acl");
    SAI_DEBUG("\t- ACL module debug commands");
    SAI_DEBUG("::debug bridge");
    SAI_DEBUG("\t- Bridge module debug commands");
    SAI_DEBUG("::debug l2mc");
    SAI_DEBUG("\t- L2MC module debug commands");
    SAI_DEBUG("::debug fdb");
    SAI_DEBUG("\t- FDB module debug commands");
    SAI_DEBUG("::debug l3");
    SAI_DEBUG("\t- L3 module debug commands");
    SAI_DEBUG("::debug lag");
    SAI_DEBUG("\t- LAG module debug commands");
    SAI_DEBUG("::debug mirror");
    SAI_DEBUG("\t- MIRROR module debug commands");
    SAI_DEBUG("::debug port");
    SAI_DEBUG("\t- PORT module debug commands");
    SAI_DEBUG("::debug qos");
    SAI_DEBUG("\t- QOS module debug commands");
    SAI_DEBUG("::debug sflow");
    SAI_DEBUG("\t- SFLOW (sample packet) module debug commands");
    SAI_DEBUG("::debug stp");
    SAI_DEBUG("\t- STP module debug commands");
    SAI_DEBUG("::debug vlan");
    SAI_DEBUG("\t- VLAN module debug commands");

    return;
}

static void sai_shell_debug(std_parsed_string_t handle)
{
    const char *token = NULL;

    if((token = std_parse_string_at(handle,0)) != NULL) {
        if(!strcmp(token,"help")) {
            sai_shell_debug_help();
        } else if(!strcmp(token,"vlan")) {
            sai_shell_debug_vlan(handle);
        } else if(!strcmp(token,"stp")) {
            sai_shell_debug_stp(handle);
        } else if(!strcmp(token,"acl")) {
            sai_shell_debug_acl(handle);
        } else if(!strcmp(token,"port")) {
            sai_shell_debug_port(handle);
        } else if(strcmp(token,"fdb") == 0) {
            sai_shell_debug_fdb(handle);
        } else if(strcmp(token,"l3") == 0) {
            sai_shell_debug_l3(handle);
        } else if(strcmp(token,"lag") == 0) {
            sai_shell_debug_lag(handle);
        } else if(strcmp(token,"mirror") == 0) {
            sai_shell_debug_mirror(handle);
        } else if(strcmp(token,"sflow") == 0) {
            sai_shell_debug_sflow(handle);
        } else if(strcmp(token,"qos") == 0) {
            sai_shell_debug_qos(handle);
        } else if(strcmp(token,"bridge") == 0) {
            sai_shell_debug_bridge(handle);
        } else if(strcmp(token,"l2mc") == 0) {
            sai_shell_debug_l2mc(handle);
        } else {
            sai_shell_debug_help();
        }
    } else {
        sai_shell_debug_help();
    }

    return;
}

void sai_shell_add_debug_commands(void)
{
    sai_shell_cmd_add("debug",sai_shell_debug,"debug <sai-module> <sai-module-params> ...");
}
