#!/bin/bash
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

## Flushes all tables - starts with a clean slate
ebtables -F FORWARD
ebtables -F OUTPUT
ebtables -F INPUT

## Configures Bridge tables for a VM (Simulator) setup
ebtables -I FORWARD -d BGA -j DROP

#Accept ARP request on eth0 and don't copy it to NFLOG
ebtables -A OUTPUT -p ARP -o eth0+ --arp-op Request -j ACCEPT
#NFLOG to copy all ARP requests to netlink group 100
ebtables -A OUTPUT -p ARP --arp-op Request --nflog-group 100 -j ACCEPT

# stop FCoE and FIP Packets from forwarding on all ports.
ebtables -A FORWARD -p 0x8906 -j DROP
ebtables -A FORWARD -p 0x8914 -j DROP
