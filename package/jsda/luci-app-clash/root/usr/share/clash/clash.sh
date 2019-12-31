#!/bin/sh /etc/rc.common
subscribe_url=$(uci get clash.config.subscribe_url_clash 2>/dev/null)
subtype=$(uci get clash.config.subcri 2>/dev/null)
config_type=$(uci get clash.config.config_type 2>/dev/null)
REAL_LOG="/usr/share/clash/clash_real.txt"
lang=$(uci get luci.main.lang 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/sub/config.yaml" 

		  	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration..." >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "正在下载配置..." >$REAL_LOG
			fi
			sleep 1
			
	if pidof clash >/dev/null; then
		if [ $subtype == "clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
		fi
			if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration Completed" >$REAL_LOG
				sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "下载配置完成" >$REAL_LOG
				sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
			fi
		if [ $config_type == "sub" ];then 
		/etc/init.d/clash restart 2>/dev/null
		fi
	else
		if [ $subtype == "clash" ];then
			wget --no-check-certificate --user-agent="Clash/OpenWRT" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
		fi
			if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration Completed" >$REAL_LOG
				sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "下载配置完成" >$REAL_LOG
				sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
			fi
	fi

			
			
