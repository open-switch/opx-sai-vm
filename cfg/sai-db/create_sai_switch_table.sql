CREATE TABLE SAI_SWITCH_PORT_DFLT_INFO_TABLE (switch_id int, total_ports int, cpu_port_id int, port_list varchar(100), PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_L2_DFLT_INFO_TABLE (switch_id int, fdb_table_size int, stp_instance_id int, PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_L3_DFLT_INFO_TABLE (switch_id int, max_vrf int, max_ecmp_paths int, on_link_route_support bool, PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_ACL_DFLT_INFO_TABLE (switch_id int, table_min_prio int, table_max_prio int, entry_min_prio int, entry_max_prio int, PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_MISC_INFO_TABLE (switch_id int, oper_status varchar(20), switching_mode varchar(20), cntr_refresh_interval int, max_temperature int, PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_L2_PARAMS_TABLE (switch_id int, src_mac varchar(20), max_learn_limit int, bcast_cpu_flood_enable bool, mcast_cpu_flood_enable bool, PRIMARY KEY(switch_id));
CREATE TABLE SAI_SWITCH_FDB_PARAMS_TABLE (switch_id int, aging_time int, ucast_miss_pkt_action varchar(10), bcast_miss_pkt_action varchar(10), mcast_miss_pkt_action varchar(10), PRIMARY KEY(switch_id));
