subdirs=inc
data_dbdir = $(sysconfdir)/opx
data_db_DATA = cfg/sai_vm_db.cfg \
cfg/sai-db/create_sai_acl_table.sql        cfg/sai-db/drop_sai_acl_table.sql \
cfg/sai-db/create_sai_routing_table.sql    cfg/sai-db/drop_sai_routing_table.sql \
cfg/sai-db/create_sai_switching_table.sql  cfg/sai-db/drop_sai_switching_table.sql \
cfg/sai-db/create_sai_switch_table.sql     cfg/sai-db/drop_sai_switch_table.sql

#The sai-vm library
lib_LTLIBRARIES = libsai-npu.la

libsai_npu_la_SOURCES = \
src/acl/sai_vm_acl_counter.c   src/qos/sai_vm_scheduler.c           src/sai-db/sai_routing_db_api.cpp \
src/acl/sai_vm_acl_rule.c      src/qos/sai_vm_wred.c                src/sai-db/sai_switch_db_api.cpp \
src/acl/sai_vm_acl_table.c     src/routing/sai_vm_neighbor.c        src/sai-db/sai_switching_db_api.cpp \
src/hostintf/sai_vm_hostif.c   src/routing/sai_vm_next_hop.c        src/samplepacket/sai_vm_samplepacket.c \
src/mirroring/sai_vm_mirror.c  src/routing/sai_vm_next_hop_group.c  src/switching/sai_vm_fdb.c \
src/port/sai_vm_port.c         src/routing/sai_vm_rif.c             src/switching/sai_vm_lag.c \
src/qos/sai_vm_buffer.c        src/routing/sai_vm_route.c           src/switching/sai_vm_stp.c \
src/qos/sai_vm_policer.c       src/routing/sai_vm_vrf.c             src/switching/sai_vm_vlan.c \
src/qos/sai_vm_qos.c           src/sai-db/sai_acl_db_api.cpp        src/switch/sai_vm_switch.c \
src/qos/sai_vm_qos_maps.c      src/sai-db/sai_acl_db_utils.cpp      src/udf/sai_vm_udf.c \
src/qos/sai_vm_queue.c         src/sai-db/sai_db_gen_utils.cpp \
src/qos/sai_vm_sched_group.c   src/sai-db/sai_db_init.cpp \
src/sai_vm_npu_api_query.c  src/sai_vm_npu_init_config.c  src/sai_vm_shell.c src/tunnel/sai_vm_tunnel.c

libsai_npu_la_LDFLAGS = -lopx_logging -lopx_common -lopx_sai_common_utils -lopx_db_sql
libsai_npu_la_CFLAGS = -I$(top_srcdir)/opx -I$(includedir)/opx
libsai_npu_la_CXXFLAGS = -I$(top_srcdir)/opx -std=c++11  -I$(includedir)/opx
