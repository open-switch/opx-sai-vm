CREATE TABLE SAI_ACL_TABLE (table_id int, stage varchar(10), priority int, table_size int, table_group_id int, num_qualifiers int, num_entries_in_use int, num_counters_in_use int, num_udf_qualifiers int, PRIMARY KEY(table_id));
CREATE TABLE SAI_ACL_TABLE_QUALIFIER_LIST (table_id int, qualifier varchar(100), PRIMARY KEY(table_id, qualifier), FOREIGN KEY(table_id) REFERENCES SAI_ACL_TABLE(table_id) ON DELETE CASCADE);
CREATE TABLE SAI_ACL_ENTRY (entry_id int, table_id int, priority int, admin_state bool, filter_count int, action_count int, counter_id varchar(100), PRIMARY KEY(entry_id), FOREIGN KEY(table_id) REFERENCES SAI_ACL_TABLE(table_id));
CREATE TABLE SAI_ACL_ENTRY_FILTER_LIST (entry_id int, filter varchar(100), admin_state bool, match_data varchar(100), match_mask varchar(100), PRIMARY KEY(entry_id, filter), FOREIGN KEY(entry_id) REFERENCES SAI_ACL_ENTRY(entry_id) ON DELETE CASCADE);
CREATE TABLE SAI_ACL_ENTRY_ACTION_LIST (entry_id int, action varchar(100), admin_state bool, parameter varchar(100), PRIMARY KEY(entry_id, action), FOREIGN KEY(entry_id) REFERENCES SAI_ACL_ENTRY(entry_id) ON DELETE CASCADE);
CREATE TABLE SAI_ACL_COUNTER (counter_id int, table_id int, counter_type varchar(20), reference_count int, byte_count int, packet_count int, PRIMARY KEY(counter_id), FOREIGN KEY(table_id) REFERENCES SAI_ACL_TABLE(table_id));

PRAGMA foreign_keys = ON;
