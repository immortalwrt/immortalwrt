#!/bin/sh
enable_list=$(uci get clash.config.cus_list 2>/dev/null)
if [  $enable_list -eq 1 ];then 

if [  -d /tmp/dnsmasq.clash ];then 
 rm -rf /tmp/dnsmasq.clash
fi

if [  -f /tmp/dnsmasq.d/custom_list.conf ];then 
  rm -rf /tmp/dnsmasq.d/custom_list.conf
fi

cutom_dns=$(uci get clash.config.custom_dns 2>/dev/null)

if [ ! -d /tmp/dnsmasq.d ];then 
 mkdir -p /tmp/dnsmasq.d
fi 

if [ ! -d /tmp/dnsmasq.clash ];then 
	mkdir -p /tmp/dnsmasq.clash
fi

awk '!/^$/&&!/^#/{printf("server=/%s/'"$cutom_dns"'\n",$0)}' /usr/share/clash/server.list >> /tmp/dnsmasq.clash/custom_list.conf
ln -s /tmp/dnsmasq.clash/custom_list.conf /tmp/dnsmasq.d/custom_list.conf
fi

