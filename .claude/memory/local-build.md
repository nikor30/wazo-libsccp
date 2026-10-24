---
name: local-build
description: How to compile wazo-libsccp on claude-control without the lab (gcc + Wazo asterisk-dev headers unpacked)
metadata:
  node_type: memory
  type: reference
  originSessionId: b761faf2-ad44-44d0-a3b9-2068eba75001
  modified: 2026-10-21T20:08:55.724Z
---

claude-control has gcc 15 and make, installed with apt on 2026-10-21. To compile:

1. Download Wazo's `asterisk-dev` .deb from mirror.wazo.community (`pelican-bookworm`, 22.11.0-1~wazo5). Take the filename from `dists/pelican-bookworm/main/binary-amd64/Packages.gz`.
2. Unpack it with `dpkg-deb -x <deb> <dir>`. Do not install it.
3. Build with `make CC="gcc -std=gnu17 -I<dir>/usr/include"`, then run `make check`.

Use `-std=gnu17` because gcc 15 defaults to C23, while Wazo builds with Debian 12's gcc 12. Build in a copy (`git archive HEAD | tar -x`) so no .o files land in the repo.

The upstream base (4cae973) already has 3 warnings: sccp_config.c format-truncation, sccp.c discarded-qualifiers, sccp_msg.c fallthrough. Related: [[sccp-lab]].
