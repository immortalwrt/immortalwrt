#!/bin/sh /etc/rc.common
. /lib/functions.sh

lang=$(uci get luci.main.lang 2>/dev/null)
loadgroups=$(uci get clash.config.loadgroups 2>/dev/null)
loadservers=$(uci get clash.config.loadservers 2>/dev/null)
load_from=$(uci get clash.config.loadfrom 2>/dev/null)


if [ "$load_from" == "sub" ];then 
        load="/usr/share/clash/config/sub/config.yaml"	
elif [ "$load_from" == "upl" ];then
	load="/usr/share/clash/config/upload/config.yaml"
fi

CFG_FILE="/etc/config/clash"
REAL_LOG="/usr/share/clash/clash_real.txt"

if [ $loadgroups -eq 1 ];then



if [ ! -f $load ] && [ ! -f $load ]; then
  exit 0
fi

	if [ $lang == "zh_cn" ];then
		echo "开始更新策略组配置..." >$REAL_LOG 
	elif [ $lang == "en" ] || [ $lang == "auto" ];then
    	echo "Start updating policy group config" >$REAL_LOG
	fi

	sleep 3
if [ -f "$load" ]; then
	 [ ! -z "$(grep "^ \{0,\}'Proxy':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"Proxy":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'Proxy\':/c\Proxy:" "$load"
	    sed -i '/^ \{0,\}\"Proxy\":/c\Proxy:' "$load"
	 }
	 
	 [ ! -z "$(grep "^ \{0,\}'Proxy Group':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"Proxy Group":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'Proxy Group\':/c\Proxy Group:" "$load"
	    sed -i '/^ \{0,\}\"Proxy Group\":/c\Proxy Group:' "$load"
	 }
	 
	 [ ! -z "$(grep "^ \{0,\}'Rule':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"Rule":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'Rule\':/c\Rule:" "$load"
	    sed -i '/^ \{0,\}\"Rule\":/c\Rule:' "$load"
	 }
	 
	 [ ! -z "$(grep "^ \{0,\}'dns':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"dns":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'dns\':/c\dns:" "$load"
	    sed -i '/^ \{0,\}\"dns\":/c\dns:' "$load"
	 }
	 
   awk '/Proxy:/,/Rule:/{print}' $load 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   if [ "$?" -eq "0" ]; then
      echo 'DIRECT' >>/tmp/Proxy_Group
      echo 'REJECT' >>/tmp/Proxy_Group
   else
      
	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Read error, configuration file exception!" >/tmp/Proxy_Group
		elif [ $lang == "zh_cn" ];then
			echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
		fi
   fi
else
	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Read error, configuration file exception!" >/tmp/Proxy_Group
		elif [ $lang == "zh_cn" ];then
			echo '读取错误，配置文件异常！' >/tmp/Proxy_Group
		fi
fi


if [ $lang == "en" ] || [ $lang == "auto" ];then
[ ! -z "$(grep "Read error" /tmp/Proxy_Group)"] && {
	echo "Read error, configuration file exception!" >$REAL_LOG
	uci commit clash
	sleep 5
	echo "" >$REAL_LOG
	exit 0
}
elif [ $lang == "zh_cn" ];then
[ ! -z "$(grep "读取错误" /tmp/Proxy_Group)"] && {
	echo "读取错误，配置文件异常！" >$REAL_LOG
	uci commit clash
	sleep 5
	echo "" >$REAL_LOG
	exit 0
}
fi
		


awk '/Proxy Group:/,/Rule:/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1


if [ -f /tmp/yaml_group.yaml ] && [ "$(ls -l /tmp/yaml_group.yaml | awk '{print int($5)}')" -eq 0 ];then

 	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "No policy group found. Aborting Operation .." >$REAL_LOG 
		sleep 5
		echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
    	 	echo "找不到策略组。中止操作..." >$REAL_LOG
		 sleep 5
		echo "Clash for OpenWRT" >$REAL_LOG
	fi
	exit 0	
else
   while [[ "$( grep -c "config groups" $CFG_FILE )" -ne 0 ]] 
   do
      uci delete clash.@groups[0] && uci commit clash >/dev/null 2>&1
   done

fi





count=1
file_count=1
match_group_file="/tmp/Proxy_Group"
group_file="/tmp/yaml_group.yaml"
line=$(sed -n '/name:/=' $group_file)
num=$(grep -c "name:" $group_file)
   
cfg_get()
{
	echo "$(grep "$1" "$2" 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null)"
}

for n in $line
do
   single_group="/tmp/group_$file_count.yaml"
   
   [ "$count" -eq 1 ] && {
      startLine="$n"
  }

   count=$(expr "$count" + 1)
   if [ "$count" -gt "$num" ]; then
      endLine=$(sed -n '$=' $group_file)
   else
      endLine=$(expr $(echo "$line" | sed -n "${count}p") - 1)
   fi
  
   sed -n "${startLine},${endLine}p" $group_file >$single_group
   startLine=$(expr "$endLine" + 1)
   
   #type
   group_type="$(cfg_get "type:" "$single_group")"
   #name
   group_name="$(cfg_get "name:" "$single_group")"
   #test_url
   group_test_url="$(cfg_get "url:" "$single_group")"
   #test_interval
   group_test_interval="$(cfg_get "interval:" "$single_group")"

   
	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Now Reading 【$group_type】-【$group_name】 Policy Group..." >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "正在读取【$group_type】-【$group_name】策略组配置..." >$REAL_LOG
		fi
		
   name=clash
   uci_name_tmp=$(uci add $name groups)
   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."
   ${uci_set}name="$group_name"
   ${uci_set}old_name="$group_name"
   ${uci_set}old_name_cfg="$group_name"
   ${uci_set}type="$group_type"
   ${uci_set}test_url="$group_test_url"
   ${uci_set}test_interval="$group_test_interval"
   
   #other_group
   cat $single_group |while read line
   do 
      if [ -z "$(echo "$line" |grep "^ \{0,\}-")" ]; then
        continue
      fi
      
      group_name1=$(echo "$line" |grep -v "name:" 2>/dev/null |grep "^ \{0,\}-" 2>/dev/null |awk -F '^ \{0,\}-' '{print $2}' 2>/dev/null |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null)
     group_name2=$(echo "$line" |awk -F 'proxies: \\[' '{print $2}' 2>/dev/null |sed 's/].*//' 2>/dev/null |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}, \{0,\}/#,#/g' 2>/dev/null)	

      if [ -z "$group_name1" ] && [ -z "$group_name2" ]; then
         continue
      elif [ ! -z "$group_name1" ] && [ -z "$group_name2" ]; then
         if [ ! -z "$(grep -F "$group_name1" $match_group_file)" ] && [ "$group_name1" != "$group_name" ]; then
            ${uci_add}other_group="$group_name1"
         fi
      elif [ -z "$group_name1" ] && [ ! -z "$group_name2" ]; then
	  
         group_num=$(( $(echo "$group_name2" | grep -o '#,#' | wc -l) + 1))
         if [ "$group_num" -le 1 ]; then
            if [ ! -z "$(grep -F "$group_name2" $match_group_file)" ] && [ "$group_name2" != "$group_name" ]; then
               ${uci_add}other_group="$group_name2"
            fi
         else
            group_nums=1
            while [[ $group_nums -le $group_num ]]
            do
               other_group_name=$(echo "$group_name2" |awk -v t="${group_nums}" -F '#,#' '{print $t}' 2>/dev/null)
               if [ ! -z "$(grep -F "$other_group_name" $match_group_file 2>/dev/null)" ] && [ "$other_group_name" != "$group_name" ]; then
                  ${uci_add}other_group="$other_group_name"
               fi
               group_nums=$(( $group_nums + 1))
            done
         fi 
		fi 
   done
   
   file_count=$(( $file_count + 1))
    
done

uci commit clash

 	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Reading Policy Group Completed" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "读取策略组配置完成" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG			
		fi

rm -rf /tmp/group_*.yaml /tmp/yaml_group.yaml /tmp/Proxy_Group 2>/dev/null
		
awk '/^ {0,}Rule:/,/^ {0,}##END/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/rule.yaml 2>&1
	rm -rf /usr/shar/clash/custom_rule.yaml 2>/dev/null
	mv /tmp/rule.yaml /usr/share/clash/custom_rule.yaml 2>/dev/null
	rm -rf /tmp/rule.yaml 2>&1  

fi



if [ $loadservers -eq 1 ];then

   while [[ "$( grep -c "config servers" $CFG_FILE )" -ne 0 ]] 
   do
      uci delete clash.@servers[0] && uci commit clash >/dev/null 2>&1
   done
   
awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1

if [ -f /tmp/yaml_proxy.yaml ] && [ "$(ls -l /tmp/yaml_proxy.yaml | awk '{print int($5)}')" -eq 0 ];then

 	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "No proxies found. Aborting Operation .." >$REAL_LOG 
		sleep 5
		echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
    	 	echo "找不到代理。中止操作..." >$REAL_LOG
		 sleep 5
		echo "Clash for OpenWRT" >$REAL_LOG
	fi
	exit 0	
else
   while [[ "$( grep -c "config servers" $CFG_FILE )" -ne 0 ]] 
   do
      uci delete clash.@servers[0] && uci commit clash >/dev/null 2>&1
   done

fi

server_file="/tmp/yaml_proxy.yaml"
single_server="/tmp/servers.yaml"
count=1
line=$(sed -n '/^ \{0,\}-/=' $server_file)
num=$(grep -c "^ \{0,\}-" $server_file)

cfg_get()
{
	echo "$(grep "$1" $single_server 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null)"
}


for n in $line
do

   [ "$count" -eq 1 ] && {
      startLine="$n"
  }

   count=$(expr "$count" + 1)
   if [ "$count" -gt "$num" ]; then
      endLine=$(sed -n '$=' $server_file)
   else
      endLine=$(expr $(echo "$line" | sed -n "${count}p") - 1)
   fi
  
   sed -n "${startLine},${endLine}p" $server_file >$single_server
   startLine=$(expr "$endLine" + 1)
   
   #type
   server_type="$(cfg_get "type:")"
   #name
   server_name="$(cfg_get "name:")"
   #server
   server="$(cfg_get "server:")"
   #port
   port="$(cfg_get "port:")"
   #cipher
   cipher="$(cfg_get "cipher:")"
   #password
   password="$(cfg_get "password:")"
   #protocol
   protocol="$(cfg_get "protocol:")"
   #protocolparam
   protocolparam="$(cfg_get "protocolparam:")"
   #obfsparam
   obfsparam="$(cfg_get "obfsparam:")"
   #udp
   udp="$(cfg_get "udp:")"
   #plugin:
   plugin="$(cfg_get "plugin:")"
   #plugin-opts:
   plugin_opts="$(cfg_get "plugin-opts:")"
   #obfs:
   obfs="$(cfg_get "obfs:")"
   #obfs-host:
   obfs_host="$(cfg_get "obfs-host:")"
   #psk:
   psk="$(cfg_get "psk:")"
   #mode:
   mode="$(cfg_get "mode:")"
   #tls:
   tls="$(cfg_get "tls:")"
   #skip-cert-verify:
   verify="$(cfg_get "skip-cert-verify:")"
   #mux:
   mux="$(cfg_get "mux:")"
   #host:
   host="$(cfg_get "host:")"
   #Host:
   Host="$(cfg_get "Host:")"
   #path:
   path="$(cfg_get "path:")"
   #ws-path:
   ws_path="$(cfg_get "ws-path:")"
   #headers_custom:
   headers="$(cfg_get "custom:")"
   #uuid:
   uuid="$(cfg_get "uuid:")"
   #alterId:
   alterId="$(cfg_get "alterId:")"
   #network
   network="$(cfg_get "network:")"
   #username
   username="$(cfg_get "username:")"
   #tls_custom:
   tls_custom="$(cfg_get "tls:")"
   
   
 	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Now Reading 【$server_type】-【$server_name】 Proxies..." >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "正在读取【$server_type】-【$server_name】代理配置..." >$REAL_LOG
		fi 
   name=clash
   uci_name_tmp=$(uci add $name servers)

   uci_set="uci -q set $name.$uci_name_tmp."
   uci_add="uci -q add_list $name.$uci_name_tmp."
    
   ${uci_set}name="$server_name"
   ${uci_set}type="$server_type"
   ${uci_set}server="$server"
   ${uci_set}port="$port"
   if [ "$server_type" = "vmess" ]; then
      ${uci_set}securitys="$cipher"
   elif [ "$server_type" = "ss" ]; then
      ${uci_set}cipher="$cipher"
   elif [ "$server_type" = "ssr" ]; then
      ${uci_set}cipher_ssr="$cipher"  
   fi
   ${uci_set}udp="$udp"
   
   ${uci_set}protocol="$protocol"
   ${uci_set}protocolparam="$protocolparam"

   if [ "$server_type" = "ss" ]; then
      ${uci_set}obfs="$obfs"
   elif [ "$server_type" = "ssr" ]; then
      ${uci_set}obfs_ssr="$obfs"
   fi
  
	
    ${uci_set}tls_custom="$tls_custom"

   ${uci_set}obfsparam="$obfsparam"

  
   ${uci_set}host="$obfs_host"
   

   [ -z "$obfs" ] && ${uci_set}obfs="$mode"

   if [ "$server_type" = "vmess" ]; then

	[ -z "$mode" ] && [ ! -z "$network" ] && ${uci_set}obfs_vmess="websocket"
	   
	[ -z "$mode" ] && [ -z "$network" ] && ${uci_set}obfs_vmess="none"
   fi
   ${uci_set}obfs_snell="$mode"
   [ -z "$obfs" ] && ${uci_set}obfs="$mode"
   [ -z "$obfs" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
   [ -z "$mode" ] && ${uci_set}obfs_snell="none"
   [ -z "$obfs_host" ] && ${uci_set}host="$host"

   if [ $tls ] && [ "$server_type" != "ss" ];then 
   ${uci_set}tls="$tls"
   fi
   ${uci_set}psk="$psk"
   if [ $verify ] && [ "$server_type" != "ssr" ];then
   ${uci_set}skip_cert_verify="$verify"
   fi

   ${uci_set}path="$path"
   [ -z "$path" ] && ${uci_set}path="$ws_path"
   ${uci_set}mux="$mux"
   ${uci_set}custom="$headers"
   
   [ -z "$headers" ] && ${uci_set}custom="$Host"
    
   if [ "$server_type" = "vmess" ]; then
    #v2ray
    ${uci_set}alterId="$alterId"
    ${uci_set}uuid="$uuid"
   fi
	
   if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
     ${uci_set}auth_name="$username"
     ${uci_set}auth_pass="$password"
   else
     ${uci_set}password="$password"
   fi
	
done

sleep 2

uci commit clash

 	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Reading Server Completed" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG			
		elif [ $lang == "zh_cn" ];then
			echo "读取代理配置完成" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG			
		fi
rm -rf /tmp/servers.yaml 2>/dev/null
rm -rf /tmp/yaml_proxy.yaml 2>/dev/null
fi


/usr/share/clash/proxy.sh 2>/dev/null

