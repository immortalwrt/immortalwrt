#!/bin/bash
subscribe_url=$(uci get clash.config.subscribe_url 2>/dev/null)
subtype=$(uci get clash.config.subcri 2>/dev/null)
urlv2ray=$(uci get clash.config.v2ray 2>/dev/null)
urlsurge=$(uci get clash.config.surge 2>/dev/null)
enable=$(uci get clash.config.enable 2>/dev/null)
if pidof clash >/dev/null; then
/etc/init.d/clash stop 2>/dev/null
fi
rm -rf /etc/clash/config.bak 2> /dev/null
if [ $subtype == "clash" ];then
wget-ssl --user-agent="User-Agent: Mozilla" $subscribe_url -O 2>&1 >1 /etc/clash/config.yaml
elif [ $subtype == "v2rayn2clash" ];then
wget-ssl --user-agent="User-Agent: Mozilla" $urlv2ray.$subscribe_url -O 2>&1 >1 /etc/clash/server.yaml
if [ -f /etc/clash/server.yaml ];then
sed -i '/Rule:/,$d' /etc/clash/server.yaml 
cat /etc/clash/server.yaml /usr/share/clash/rule.yaml > /etc/clash/config.yaml 
fi
elif [ $subtype == "surge2clash" ];then
wget-ssl --user-agent="User-Agent: Mozilla" $urlsurge.$subscribe_url -O 2>&1 >1 /etc/clash/server.yaml
if [ -f /etc/clash/server.yaml ];then
sed -i '/Rule:/,$d' /etc/clash/server.yaml 
cat /etc/clash/server.yaml /usr/share/clash/rule.yaml > /etc/clash/config.yaml
fi
fi
rm -rf /etc/clash/server.yaml 2> /dev/null
if [ $enable -eq 1 ]; then
[ "$?" -eq "0" ] && /etc/init.d/clash restart 2>/dev/null
fi

