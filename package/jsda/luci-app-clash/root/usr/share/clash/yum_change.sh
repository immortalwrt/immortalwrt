#!/bin/sh
CONFIG_YAML="/etc/clash/config.yaml"
CONFIG_YAML_SUB="/usr/share/clash/config/sub/config.yaml"
CONFIG_YAML_UPL="/usr/share/clash/config/upload/config.yaml"
CONFIG_YAML_CUS="/usr/share/clash/config/custom/config.yaml"

config_type=$(uci get clash.config.config_type 2>/dev/null)

if [ $config_type == "sub" ];then 
if [  -f $CONFIG_YAML_SUB ] && [ "$(ls -l $CONFIG_YAML_SUB|awk '{print int($5/1024)}')" -ne 0 ];then
	cp $CONFIG_YAML_SUB $CONFIG_YAML
fi
elif [ $config_type == "upl" ];then 
if [  -f $CONFIG_YAML_UPL ] && [ "$(ls -l $CONFIG_YAML_UPL|awk '{print int($5/1024)}')" -ne 0 ];then
	cp $CONFIG_YAML_UPL $CONFIG_YAML
fi
elif [ $config_type == "cus" ];then 
if [  -f $CONFIG_YAML_CUS ] && [ "$(ls -l $CONFIG_YAML_CUS|awk '{print int($5/1024)}')" -ne 0 ];then
	cp $CONFIG_YAML_CUS $CONFIG_YAML
fi
fi

if [  -f $CONFIG_YAML ];then

if [ -z "$(grep "^ \{0,\}listen:" $CONFIG_YAML)" ] || [ -z "$(grep "^ \{0,\}enhanced-mode:" $CONFIG_YAML)" ] || [ -z "$(grep "^ \{0,\}enable:" $CONFIG_YAML)" ] || [ -z "$(grep "^ \{0,\}dns:" $CONFIG_YAML)" ] ;then
#===========================================================================================================================
	uci set clash.config.mode="1" && uci commit clash
#===========================================================================================================================	
fi
 

#===========================================================================================================================
		mode=$(uci get clash.config.mode 2>/dev/null)
		da_password=$(uci get clash.config.dash_pass 2>/dev/null)
		redir_port=$(uci get clash.config.redir_port 2>/dev/null)
		http_port=$(uci get clash.config.http_port 2>/dev/null)
		socks_port=$(uci get clash.config.socks_port 2>/dev/null)
		dash_port=$(uci get clash.config.dash_port 2>/dev/null)
		bind_addr=$(uci get clash.config.bind_addr 2>/dev/null)
		allow_lan=$(uci get clash.config.allow_lan 2>/dev/null)
		log_level=$(uci get clash.config.level 2>/dev/null)
		subtype=$(uci get clash.config.subcri 2>/dev/null)
				
if [ $mode -eq 1 ];  then	
		
		sed -i "/Proxy:/i\#clash-openwrt" $CONFIG_YAML
                sed -i "/#clash-openwrt/a\#=============" $CONFIG_YAML
		sed -i "/#=============/a\ " $CONFIG_YAML
		sed -i '1,/#clash-openwrt/d' $CONFIG_YAML		
		mv /etc/clash/config.yaml /etc/clash/dns.yaml
		cat /usr/share/clash/dns.yaml /etc/clash/dns.yaml > $CONFIG_YAML 
		rm -rf /etc/clash/dns.yaml
		sed -i "1i\port: ${http_port}" $CONFIG_YAML
		sed -i "2i\socks-port: ${socks_port}" $CONFIG_YAML
		sed -i "3i\redir-port: ${redir_port}" $CONFIG_YAML
		sed -i "4i\allow-lan: ${allow_lan}" $CONFIG_YAML
		if [ $allow_lan == "true" ];  then	
		sed -i "5i\bind-address: '${bind_addr}'" $CONFIG_YAML
		else
		sed -i "5i\#bind-address: " $CONFIG_YAML
		fi
		sed -i "6i\mode: Rule" $CONFIG_YAML
		sed -i "7i\log-level: ${log_level}" $CONFIG_YAML
		sed -i "8i\external-controller: 0.0.0.0:${dash_port}" $CONFIG_YAML
		sed -i "9i\secret: '${da_password}'" $CONFIG_YAML
		sed -i "10i\external-ui: "/usr/share/clash/dashboard"" $CONFIG_YAML
		sed -i "11i\ " $CONFIG_YAML
		sed -i "12i\ " $CONFIG_YAML
		sed -i '/#=============/ d' $CONFIG_YAML		
else
		if [ $subtype == "v2rayn2clash" ];then
		sed -i "/Proxy:/i\#clash-openwrt" $CONFIG_YAML
                sed -i "/#clash-openwrt/a\#=============" $CONFIG_YAML
		sed -i "/#=============/a\ " $CONFIG_YAML
		sed -i '1,/#clash-openwrt/d' $CONFIG_YAML		
		mv $CONFIG_YAML /etc/clash/dns.yaml
		cat /usr/share/clash/dns.yaml /etc/clash/dns.yaml > $CONFIG_YAML 
		rm -rf /etc/clash/dns.yaml

		else

		sed -i "/dns:/i\#clash-openwrt" $CONFIG_YAML
               sed -i "/#clash-openwrt/a\#=============" $CONFIG_YAML
		sed -i '1,/#clash-openwrt/d' $CONFIG_YAML
		fi

		sed -i "1i\port: ${http_port}" $CONFIG_YAML
		sed -i "2i\socks-port: ${socks_port}" $CONFIG_YAML
		sed -i "3i\redir-port: ${redir_port}" $CONFIG_YAML
		sed -i "4i\allow-lan: ${allow_lan}" $CONFIG_YAML
		if [ $allow_lan == "true" ];  then	
		sed -i "5i\bind-address: '${bind_addr}'" $CONFIG_YAML
		else
		sed -i "5i\#bind-address: " $CONFIG_YAML
		fi
		sed -i "6i\mode: Rule" $CONFIG_YAML
		sed -i "7i\log-level: ${log_level}" $CONFIG_YAML
		sed -i "8i\external-controller: 0.0.0.0:${dash_port}" $CONFIG_YAML
		sed -i "9i\secret: '${da_password}'" $CONFIG_YAML
		sed -i "10i\external-ui: "/usr/share/clash/dashboard"" $CONFIG_YAML
		sed -i "11i\ " $CONFIG_YAML
		sed -i "12i\ " $CONFIG_YAML
		sed -i '/#=============/ d' $CONFIG_YAML
fi
#=========================================================================================================================== 
fi