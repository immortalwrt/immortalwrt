#!/bin/bash
subscribe_url=$(uci get clash.config.subscribe_url 2>/dev/null)
rm -rf /etc/clash/config.bak 2> /dev/null
wget-ssl --user-agent="User-Agent: Mozilla" $subscribe_url -O 2>&1 >1 /etc/clash/config.yml
[ "$?" -ne "0" ] && wget-ssl -O 2>&1 >1 /etc/clash/config.yml "$subscribe_url"
[ "$?" -eq "0" ] && /etc/init.d/clash restart 2>/dev/null
