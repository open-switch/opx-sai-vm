#
# Copyright (c) 2018 Dell Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may
# not use this file except in compliance with the License. You may obtain
# a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
#
# THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
# CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
# LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
# FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
#
# See the Apache Version 2.0 License for specific language governing
# permissions and limitations under the License.
#

subdirs=inc
data_dbdir = $(sysconfdir)/opx
bin_SCRIPTS = cfg/vm_dn_rules.sh
systemdconfdir=/lib/systemd/system
systemdconf_DATA = systemd/*.service
data_db_DATA = cfg/sai_vm_db.cfg \
cfg/sai-db/create_sai_acl_table.sql        cfg/sai-db/drop_sai_acl_table.sql \
cfg/sai-db/create_sai_routing_table.sql    cfg/sai-db/drop_sai_routing_table.sql \
cfg/sai-db/create_sai_switching_table.sql  cfg/sai-db/drop_sai_switching_table.sql \
cfg/sai-db/create_sai_switch_table.sql     cfg/sai-db/drop_sai_switch_table.sql

#The sai-vm library
lib_LTLIBRARIES = libsai-0.9.6.la

libsai_0_9_6_la_SOURCES = \
src/acl/sai_vm_acl_counter.c   src/qos/sai_vm_scheduler.c           src/sai-db/sai_routing_db_api.cpp \
src/acl/sai_vm_acl_rule.c      src/qos/sai_vm_wred.c                src/sai-db/sai_switch_db_api.cpp \
src/acl/sai_vm_acl_table.c     src/acl/sai_vm_acl_range.c src/routing/sai_vm_neighbor.c        src/sai-db/sai_switching_db_api.cpp \
src/hostintf/sai_vm_hostif.c   src/routing/sai_vm_next_hop.c        src/samplepacket/sai_vm_samplepacket.c \
src/mirroring/sai_vm_mirror.c  src/routing/sai_vm_next_hop_group.c  src/switching/sai_vm_fdb.c \
src/port/sai_vm_port.c         src/routing/sai_vm_rif.c             src/switching/sai_vm_lag.c \
src/qos/sai_vm_buffer.c        src/routing/sai_vm_route.c           src/switching/sai_vm_stp.c \
src/qos/sai_vm_policer.c       src/routing/sai_vm_vrf.c             src/switching/sai_vm_vlan.c \
src/qos/sai_vm_qos.c           src/sai-db/sai_acl_db_api.cpp        src/switch/sai_vm_switch.c \
src/qos/sai_vm_qos_maps.c      src/sai-db/sai_acl_db_utils.cpp \
src/udf/sai_vm_udf.c           src/qos/sai_vm_queue.c               src/sai-db/sai_db_gen_utils.cpp \
src/qos/sai_vm_sched_group.c   src/sai-db/sai_db_init.cpp \
src/sai_vm_npu_api_query.c  src/sai_vm_npu_init_config.c  src/sai_vm_shell.c src/tunnel/sai_vm_tunnel.c \
src/bridge/sai_vm_bridge.c \
src/fc/sai_vm_fc_port.c \
src/fc/sai_vm_fc_switch.c \
src/switching/sai_vm_l2mc.c \
src/switching/sai_vm_mcast.c \
	src/acl/sai_acl_counter.c \
	src/acl/sai_acl_debug.c \
	src/acl/sai_acl_init.c \
	src/acl/sai_acl_policer.c \
	src/acl/sai_acl_range.c \
	src/acl/sai_acl_rule.c \
	src/acl/sai_acl_rule_utils.c \
	src/acl/sai_acl_table.c \
	src/acl/sai_acl_table_group.c \
	src/acl/sai_acl_table_group_member.c \
	src/acl/sai_acl_utils.c \
	src/bridge/sai_bridge.c \
	src/bridge/sai_bridge_db.cpp \
	src/bridge/sai_bridge_debug.c \
	src/bridge/sai_bridge_port.c \
	src/bridge/sai_bridge_utils.c \
	src/hostintf/sai_hostintf.c \
	src/hostintf/sai_hostintf_debug.c \
	src/hostintf/sai_hostintf_utils.c \
	src/mirroring/sai_mirror_common.c \
	src/mirroring/sai_mirror_debug.c \
	src/mirroring/sai_mirror_port.c \
	src/mirroring/sai_mirror_utils.c \
	src/port/sai_port.c \
	src/port/sai_port_attributes.c \
	src/port/sai_port_debug.c \
	src/port/sai_port_utils.c \
	src/qos/sai_qos_buffer.c \
	src/qos/sai_qos_buffer_common.c \
	src/qos/sai_qos_buffer_util.c \
	src/qos/sai_qos_debug.c \
	src/qos/sai_qos_hierarchy.c \
	src/qos/sai_qos_init.c \
	src/qos/sai_qos_map_utils.c \
	src/qos/sai_qos_maps.c \
	src/qos/sai_qos_maps_debug.c \
	src/qos/sai_qos_pfc_maps.c \
	src/qos/sai_qos_pg.c \
	src/qos/sai_qos_policer.c \
	src/qos/sai_qos_policer_debugs.c \
	src/qos/sai_qos_port.c \
	src/qos/sai_qos_port_util.c \
	src/qos/sai_qos_queue.c \
	src/qos/sai_qos_sched_group.c \
	src/qos/sai_qos_scheduler.c \
	src/qos/sai_qos_scheduler_util.c \
	src/qos/sai_qos_util.c \
	src/qos/sai_qos_wred.c \
	src/qos/sai_qos_wred_debugs.c \
	src/routing/sai_l3_debug.c \
	src/routing/sai_l3_encap_next_hop.c \
	src/routing/sai_l3_init.c \
	src/routing/sai_l3_ipmc.c \
	src/routing/sai_l3_ipmc_group.c \
	src/routing/sai_l3_ipmc_repl_group.c \
	src/routing/sai_l3_ipmc_rpf_group.c \
	src/routing/sai_vm_l3_ipmc.c \
	src/routing/sai_vm_l3_mcast.c \
	src/routing/sai_l3_mem.c \
	src/routing/sai_l3_neighbor.c \
	src/routing/sai_l3_next_hop.c \
	src/routing/sai_l3_next_hop_group.c \
	src/routing/sai_l3_next_hop_group_utl.c \
	src/routing/sai_l3_rif_utils.c \
	src/routing/sai_l3_route.c \
	src/routing/sai_l3_router_interface.c \
	src/routing/sai_l3_util.c \
	src/routing/sai_l3_vrf.c \
	src/sai_gen_utils.c \
	src/sai_map_utl.cpp \
	src/switching/sai_fdb.c \
	src/switching/sai_fdb_debug.c \
	src/switching/sai_fdb_utils.c \
	src/switching/sai_l2mc.c \
	src/switching/sai_l2mc_debug.c \
	src/switching/sai_l2mc_group.c \
	src/switching/sai_l2mc_utils.c \
	src/switching/sai_lag.c \
	src/switching/sai_lag_debug.c \
	src/switching/sai_lag_utils.c \
	src/switching/sai_mcast_utils.c \
	src/switching/sai_stp.c \
	src/switching/sai_stp_debug.c \
	src/switching/sai_stp_utils.c \
	src/switching/sai_vlan.c \
	src/switching/sai_vlan_debug.c \
	src/switching/sai_vlan_utils.c \
	src/tunnel/sai_tunnel_encap_nh.c \
	src/tunnel/sai_tunnel_map_obj.c \
	src/tunnel/sai_tunnel_obj.c \
	src/tunnel/sai_tunnel_term_obj.c \
	src/tunnel/sai_tunnel_utils.c \
        src/vport/sai_vm_vport.cpp \
        src/vport/sai_vm_vport_event.c \
        src/hash/sai_hash_obj.c \
        src/switchinfra/sai_extn_api_query.c \
        src/switchinfra/sai_switch.c \
        src/switchinfra/sai_switch_init_config.c \
        src/switchinfra/sai_func_query.c \
        src/switchinfra/sai_switch_debug.c \
        src/switchinfra/sai_switch_utils.c \
        src/shell/sai_shell.c \
        src/shell/sai_shell_debug_handler.c \
        src/shell/sai_shell_init.c \
        src/samplepacket/sai_samplepacket_utils.c \
        src/samplepacket/sai_samplepacket_port.c \
        src/samplepacket/sai_samplepacket_debug.c \
        src/samplepacket/sai_samplepacket_common.c \
        src/udf/sai_udf.c \
        src/udf/sai_udf_group.c \
        src/udf/sai_udf_utils.c \
        src/acl/sai_acl_slice.c \
        src/acl/sai_vm_acl_slice.c


libsai_0_9_6_la_LDFLAGS = -lopx_logging -lopx_common -lopx_db_sql -version-info 1:0:0
libsai_0_9_6_la_CFLAGS = -I$(top_srcdir)/opx -I$(includedir)/opx
libsai_0_9_6_la_CXXFLAGS = -I$(top_srcdir)/opx -std=c++11  -I$(includedir)/opx
