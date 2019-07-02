#!/bin/sh

pdnsd_port="5353"
awk '!/^$/&&!/^#/{printf("ipset=/.%s/'"gfwlist"'\n",$0)}' /etc/config/clash_gfw.list > /etc/clash/custom_forward.conf
awk '!/^$/&&!/^#/{printf("server=/.%s/'"127.0.0.1#$pdnsd_port"'\n",$0)}' /etc/config/clash_gfw.list >> /etc/clash/custom_forward.conf

