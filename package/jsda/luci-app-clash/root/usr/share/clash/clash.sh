#!/bin/bash
subscribe_url=$(uci get clash.config.subscribe_url 2>/dev/null)
subtype=$(uci get clash.config.subcri 2>/dev/null)
urlv2ray=$(uci get clash.config.v2ray 2>/dev/null)
urlsurge=$(uci get clash.config.surge 2>/dev/null)
CONFIG_YAML="/etc/clash/config.yaml"
CONFIG_YAML_TEMP="/etc/clash/server.yaml"
CONFIG_YAML_RULE="/usr/share/clash/rule.yaml"
if pidof clash >/dev/null; then
/etc/init.d/clash stop 2>/dev/null
rm -rf /etc/clash/config.bak 2> /dev/null
if [ $subtype == "clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
elif [ $subtype == "v2rayn2clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML_TEMP
if [ -f $CONFIG_YAML_TEMP ];then
sed -i '/Rule:/,$d' $CONFIG_YAML_TEMP 
cat $CONFIG_YAML_TEMP $CONFIG_YAML_RULE > $CONFIG_YAML 
fi
elif [ $subtype == "surge2clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $urlsurge.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
fi
rm -rf $CONFIG_YAML_TEMP 2> /dev/null
uci set clash.config.enable=1 2> /dev/null
uci commit clash 2> /dev/null
/etc/init.d/clash restart 2>/dev/null
else
rm -rf /etc/clash/config.bak 2> /dev/null
if [ $subtype == "clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
elif [ $subtype == "v2rayn2clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML_TEMP
if [ -f $CONFIG_YAML_TEMP ];then
sed -i '/Rule:/,$d' $CONFIG_YAML_TEMP 
cat $CONFIG_YAML_TEMP $CONFIG_YAML_RULE > $CONFIG_YAML 
fi
elif [ $subtype == "surge2clash" ];then
wget-ssl --timeout=30 --tries=2 --user-agent="User-Agent: Mozilla" $urlsurge.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
fi
rm -rf $CONFIG_YAML_TEMP 2> /dev/null
fi

