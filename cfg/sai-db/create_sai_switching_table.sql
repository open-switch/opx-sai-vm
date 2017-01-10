CREATE TABLE SAI_FDB (mac_address varchar(100), vlan_id int, IS_STATIC bool, PORT_ID int, PACKET_ACTION varchar(10), FDB_METADATA int, PRIMARY KEY(mac_address, vlan_id));
CREATE TABLE SAI_VLAN (vlan_id int, STP_INSTANCE int, MAX_LEARN_LIMIT int, DISABLE_LEARN bool, VLAN_METADATA int, PRIMARY KEY(vlan_id));
CREATE TABLE SAI_VLAN_PORT_LIST (vlan_id int, PORT_ID int, TAGGING_MODE varchar(20), PRIMARY KEY(vlan_id, PORT_ID),
FOREIGN KEY(vlan_id) REFERENCES SAI_VLAN(vlan_id) ON DELETE CASCADE);

PRAGMA foreign_keys = ON;
