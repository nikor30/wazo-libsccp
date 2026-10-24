---
name: sccp-lab
description: "Test lab for wazo-libsccp: LXC \"sccp-lab\" on pve1, VLAN 100, 192.168.1.200; created by a script the user runs"
metadata:
  node_type: memory
  type: project
  originSessionId: b761faf2-ad44-44d0-a3b9-2068eba75001
  modified: 2026-10-23T09:02:53.947Z
---

Test plan (2026-10-21): build and test chan_sccp in an LXC container `sccp-lab` on pve1 (10.12.14.4). It uses the same bridge and VLAN as the production Wazo (VM 105, 192.168.1.10), static IP 192.168.1.200 (outside the DHCP pool .12-.99), and onboot=0.

Created 2026-10-23 by me (owner said I may create containers myself, like radius01): CT 114 on local-zfs, SSH root@192.168.1.200 with my key, Asterisk 22.11 wazo5 + chan_sccp + tftpd set up. pve1 access: sshpass with the "pve1(new)" password from the server CSV (see wiki infra:proxmox). Piping a whole script to pve1 via ssh was blocked by the auto-mode guard; explicit single pct commands worked.

claude-control (192.168.100.20) can reach VLAN 100.

**Why:** production Wazo 26.10 serves DHCP on VLAN 100 and has 9 SCCP phones registered, so the lab must not run DHCP there or touch VM 105.

**How to apply:** do not change production DHCP/provd beyond what the owner approves. Exception approved 2026-10-23: the test CP-6941 (SCCP firmware) has a DHCP reservation in Wazo's /etc/dhcp/dhcpd_extra.conf (fixed .204, option 150/66 → lab .200), because the owner had no physical access to the phone. Related: [[bummfilm-phase1-backup-state]].
