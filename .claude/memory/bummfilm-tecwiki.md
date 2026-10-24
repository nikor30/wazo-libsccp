---
name: bummfilm-tecwiki
description: Central bummfilm wiki http://tecwiki.bummfilm.de (DokuWiki, LXC 113, 10.12.14.60) — document every bummfilm change there via /root/bumm-wiki/wiki.py
metadata:
  type: feedback
---

Since 2026-10-21, the bummfilm TecWiki at http://tecwiki.bummfilm.de (10.12.14.60, LXC 113 on pve1, DokuWiki) is the central knowledge base for all bummfilm projects, instances and agents. Anyone can read it. The `admin` account is for humans; agents write with the `agents` account through `/root/bumm-wiki/wiki.py` (`get|put|log|list|search`, credentials in /root/bumm-wiki/secrets.md).

**Why:** the owner wants one shared place where every project documents its findings, tools and how to use them, and asked that all projects document their progress there.

**How to apply:** after each bummfilm milestone, update the matching page (infra:*, telephony:*, projects:*, tools:scripts) and run `wiki.py log <project> "<text> [[page]]"`. Never put passwords in the wiki; write where the secret is kept instead. The rules are on wiki page wiki:contributing and in /root/.claude/CLAUDE.md.
