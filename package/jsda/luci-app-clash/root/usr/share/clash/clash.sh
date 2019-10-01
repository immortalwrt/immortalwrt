#!/bin/sh
subscribe_url=$(uci get clash.config.subscribe_url 2>/dev/null)
subtype=$(uci get clash.config.subcri 2>/dev/null)
urlv2ray=$(uci get clash.config.v2ray 2>/dev/null)
urlsurge=$(uci get clash.config.surge 2>/dev/null)
cusrule=$(uci get clash.config.cusrule 2>/dev/null)
config_type=$(uci get clash.config.config_type 2>/dev/null)


CONFIG_YAML="/usr/share/clash/config/sub/config.yaml"
CONFIG_YAML_TEMP="/etc/clash/server.yaml"
CONFIG_YAML_RULE="/usr/share/clash/rule.yaml"
 


	if pidof clash >/dev/null; then
		if [ $subtype == "clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
		elif [ $subtype == "v2rayn2clash" ];then
			if [ $cusrule == 1 ];then
				wget --no-check-certificate --user-agent="Clash/OpenWRT" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML_TEMP
				if [ -f $CONFIG_YAML_TEMP ];then
					sed -i '/Rule:/,$d' $CONFIG_YAML_TEMP 
					cat $CONFIG_YAML_TEMP $CONFIG_YAML_RULE > $CONFIG_YAML 
				fi
			else
				wget --no-check-certificate --user-agent="Clash/OpenWRT" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
			fi
		elif [ $subtype == "surge2clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $urlsurge.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
		fi
		rm -rf $CONFIG_YAML_TEMP 2>/dev/null
		if [ $config_type == "sub" ];then 
		/etc/init.d/clash restart 2>/dev/null
		fi
	else
		if [ $subtype == "clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
		elif [ $subtype == "v2rayn2clash" ];then
			if [ $cusrule == 1 ];then
				wget --no-check-certificate --user-agent="Clash/OpenWRT" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML_TEMP
				if [ -f $CONFIG_YAML_TEMP ];then
					sed -i '/Rule:/,$d' $CONFIG_YAML_TEMP 
					cat $CONFIG_YAML_TEMP $CONFIG_YAML_RULE > $CONFIG_YAML 
				fi
			else
			wget --no-check-certificate --user-agent="Clash-/OpenWRT" $urlv2ray.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
		fi
		elif [ $subtype == "surge2clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $urlsurge.$subscribe_url -O 2>&1 >1 $CONFIG_YAML
		fi
		rm -rf $CONFIG_YAML_TEMP 2>/dev/null
	fi



