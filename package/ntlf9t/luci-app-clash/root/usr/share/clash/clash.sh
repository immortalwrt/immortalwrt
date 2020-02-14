#!/bin/bash /etc/rc.common
. /lib/functions.sh
   
  
subscribe_url=$(uci get clash.config.subscribe_url_clash 2>/dev/null)
config_name=$(uci get clash.config.config_name 2>/dev/null) 
subtype=$(uci get clash.config.subcri 2>/dev/null) 
REAL_LOG="/usr/share/clash/clash_real.txt" 
lang=$(uci get luci.main.lang 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/sub/${config_name}.yaml" 
 
if  [ $config_name == "" ] || [ -z $config_name ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Tag Your Config" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "标记您的配置" >$REAL_LOG
	fi
	sleep 5
	echo "Clash for OpenWRT" >$REAL_LOG
	exit 0	
	
fi


 
check_name=$(grep -F "${config_name}.yaml" "/usr/share/clashbackup/confit_list.conf")
if [ ! -z $check_name ];then
   
	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Config with same name exist, please rename and download again" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "已存在同名配置，请重命名名配置重新下载" >$REAL_LOG
	fi
	sleep 5
	echo "Clash for OpenWRT" >$REAL_LOG
	exit 0	

   
else

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始下载配置" >$REAL_LOG
	fi
	sleep 1
			
	wget -c4 --no-check-certificate --user-agent="Clash/OpenWRT" $subscribe_url -O 2>&1 >1 $CONFIG_YAML
	if [ "$?" -eq "0" ]; then
	echo "${config_name}.yaml#$subscribe_url#$subtype" >>/usr/share/clashbackup/confit_list.conf
   
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
   
fi   
