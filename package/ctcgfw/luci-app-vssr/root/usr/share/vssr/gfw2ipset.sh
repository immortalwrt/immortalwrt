#!/bin/sh

mkdir -p /tmp/dnsmasq.vssr

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"gfwlist"'\n",$0)}' /etc/config/gfw.list > /tmp/dnsmasq.vssr/custom_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/config/gfw.list >> /tmp/dnsmasq.vssr/custom_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"blacklist"'\n",$0)}' /etc/config/black.list > /tmp/dnsmasq.vssr/blacklist_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#5335"'\n",$0)}' /etc/config/black.list >> /tmp/dnsmasq.vssr/blacklist_forward.conf

awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"whitelist"'\n",$0)}' /etc/config/white.list > /tmp/dnsmasq.vssr/whitelist_forward.conf

