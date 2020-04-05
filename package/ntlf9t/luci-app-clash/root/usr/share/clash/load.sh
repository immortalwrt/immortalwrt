#!/bin/bash /etc/rc.common

lang=$(uci get luci.main.lang 2>/dev/null)
loadgroups=$(uci get clash.config.loadgroups 2>/dev/null)
loadservers=$(uci get clash.config.loadservers 2>/dev/null)
loadprovider=$(uci get clash.config.loadprovider 2>/dev/null)

run_load(){

load="/etc/clash/config.yaml"
CONFIG_YAML_PATH=$(uci get clash.config.use_config 2>/dev/null)

if [  -f $CONFIG_YAML_PATH ] && [ "$(ls -l $CONFIG_YAML_PATH|awk '{print int($5)}')" -ne 0 ];then
	cp $CONFIG_YAML_PATH $load 2>/dev/null		
fi

if [ ! -f $load ] || [ "$(ls -l $load|awk '{print int($5)}')" -eq 0 ]; then 
  exit 0
fi 



CFG_FILE="/etc/config/clash"
REAL_LOG="/usr/share/clash/clash_real.txt"


rm -rf /tmp/Proxy_Group /tmp/servers.yaml /tmp/yaml_proxy.yaml /tmp/group_*.yaml /tmp/yaml_group.yaml /tmp/match_servers.list /tmp/yaml_provider.yaml /tmp/provider.yaml /tmp/provider_gen.yaml /tmp/provider_che.yaml /tmp/match_provider.list 2>/dev/null


	sed -i "/^ \{0,\}proxy-groups:/c\Proxy Group:" "$load" 2>/dev/null

	sed -i "/^ \{0,\}proxy-providers:/c\proxy-provider:" "$load" 2>/dev/null

	sed -i "s/^proxies:/Proxy:/" "$load" 2>/dev/null

	sed -i "/^ \{0,\}rules:/c\Rule:" "$load" 2>/dev/null
		
	 [ ! -z "$(grep "^ \{0,\}'Proxy':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"Proxy":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'Proxy\':/c\Proxy:" "$load"
	    sed -i '/^ \{0,\}\"Proxy\":/c\Proxy:' "$load"
	 }
	 
	 [ ! -z "$(grep "^ \{0,\}'proxy-provider':" "$load")" ] || [ ! -z "$(grep '^ \{0,\}"proxy-provider":' "$load")" ] && {
	    sed -i "/^ \{0,\}\'proxy-provider\:'/c\proxy-provider:" "$load"
	    sed -i '/^ \{0,\}\"proxy-provider\":/c\proxy-provider:' "$load"
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
	 
   #awk '/Proxy:/,/Rule:/{print}' $load 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   
   group_len=$(sed -n '/^ \{0,\}Proxy Group:/=' "$load" 2>/dev/null)
   provider_len=$(sed -n '/^ \{0,\}proxy-provider:/=' "$load" 2>/dev/null)
   if [ "$provider_len" -ge "$group_len" ]; then
       awk '/Proxy:/,/proxy-provider:/{print}' "$load" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
       sed -i "s/proxy-provider://g" /tmp/Proxy_Group 2>&1
   else
       awk '/Proxy:/,/Rule:/{print}' "$load" 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed 's/\t/ /g' 2>/dev/null |grep name: |awk -F 'name:' '{print $2}' |sed 's/,.*//' |sed 's/^ \{0,\}//' 2>/dev/null |sed 's/ \{0,\}$//' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null >/tmp/Proxy_Group 2>&1
   fi  
   
   
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



#awk '/Proxy Group:/,/Rule:/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1

group_len=$(sed -n '/^ \{0,\}Proxy Group:/=' "$load" 2>/dev/null)
provider_len=$(sed -n '/^ \{0,\}proxy-provider:/=' "$load" 2>/dev/null)
if [ "$provider_len" -ge "$group_len" ]; then
   awk '/Proxy Group:/,/proxy-provider:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1
   sed -i "s/proxy-provider://g" /tmp/yaml_group.yaml 2>&1
else
   awk '/Proxy Group:/,/Rule:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_group.yaml 2>&1
fi


#######READ GROUPS START
if [ $loadgroups -eq 1 ];then

if [ -f /tmp/yaml_group.yaml ];then
	while [[ "$( grep -c "config groups" $CFG_FILE )" -ne 0 ]] 
	do
      uci delete clash.@groups[0] && uci commit clash >/dev/null 2>&1
	done



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
	  proxies_len=$(sed -n '/proxies:/=' $single_group 2>/dev/null)
      use_len=$(sed -n '/use:/=' $single_group 2>/dev/null)
      name1_len=$(sed -n "/${group_name1}/=" $single_group 2>/dev/null)
      name2_len=$(sed -n "/${group_name2}/=" $single_group 2>/dev/null)
	  

      if [ -z "$group_name1" ] && [ -z "$group_name2" ]; then
         continue
      fi

      if [ ! -z "$group_name1" ] && [ -z "$group_name2" ]; then
         if [ "$proxies_len" -le "$use_len" ]; then
            if [ "$name1_len" -le "$use_len" ] && [ ! -z "$(grep -F "$group_name1" $match_group_file)" ] && [ "$group_name1" != "$group_name" ]; then
               ${uci_add}other_group="$group_name1"
            fi
         else
            if [ "$name1_len" -ge "$proxies_len" ] && [ ! -z "$(grep -F "$group_name1" $match_group_file)" ] && [ "$group_name1" != "$group_name" ]; then
               ${uci_add}other_group="$group_name1"
            fi
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


		
awk '/^ {0,}Rule:/,/^ {0,}##END/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/rule.yaml 2>&1
rm -rf /usr/shar/clash/custom_rule.yaml 2>/dev/null
mv /tmp/rule.yaml /usr/share/clash/custom_rule.yaml 2>/dev/null
rm -rf /tmp/rule.yaml 2>&1  

fi
fi
#######READ GROUPS END

   
#awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' $load 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1



 
proxy_len=$(sed -n '/^ \{0,\}Proxy:/=' $load 2>/dev/null)
group_len=$(sed -n '/^ \{0,\}Proxy Group:/=' "$load" 2>/dev/null)
provider_len=$(sed -n '/^ \{0,\}proxy-provider:/=' $load 2>/dev/null)

if [ ! -z "$provider_len" ] && [ "$provider_len" -ge "$proxy_len" ] && [ "$provider_len" -le "$group_len" ]; then
   awk '/^ {0,}Proxy:/,/^ {0,}proxy-provider:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Proxy Group:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
   sed -i '/proxy-provider:/,$d' /tmp/yaml_proxy.yaml 2>&1
   sed -i '/Proxy Group:/,$d' /tmp/yaml_provider.yaml 2>&1	
   
elif [ ! -z "$provider_len" ] && [ "$provider_len" -le "$proxy_len" ] && [ "$provider_len" -le "$group_len" ]; then
   awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Proxy:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
   sed -i '/Proxy:/,$d' /tmp/yaml_provider.yaml 2>&1
   sed -i '/Proxy Group:/,$d' /tmp/yaml_proxy.yaml 2>&1
   
elif [ ! -z "$provider_len" ] && [ "$provider_len" -ge "$group_len" ]; then
   awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   awk '/^ {0,}proxy-provider:/,/^ {0,}Rule:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_provider.yaml 2>&1
   sed -i '/Proxy Group:/,$d' /tmp/yaml_proxy.yaml 2>&1
   sed -i '/Rule:/,$d' /tmp/yaml_provider.yaml 2>&1

else
   awk '/^ {0,}Proxy:/,/^ {0,}Proxy Group:/{print}' "$load" 2>/dev/null |sed 's/\"//g' 2>/dev/null |sed "s/\'//g" 2>/dev/null |sed 's/\t/ /g' 2>/dev/null >/tmp/yaml_proxy.yaml 2>&1
   sed -i '/Proxy Group:/,$d' /tmp/yaml_proxy.yaml 2>&1
fi


match_provider="/tmp/match_provider.list"
single_provider="/tmp/provider.yaml"
single_provider_gen="/tmp/provider_gen.yaml"
single_provider_che="/tmp/provider_che.yaml"
provider_file="/tmp/yaml_provider.yaml"
group_num=$(grep -c "name:" /tmp/yaml_group.yaml)
match_servers="/tmp/match_servers.list"

server_file="/tmp/yaml_proxy.yaml"
single_server="/tmp/servers.yaml"


line=$(sed -n '/name:/=' $server_file 2>/dev/null)
num=$(grep -c "name:" $server_file 2>/dev/null)
count=1

sed -i '/^ *$/d' $provider_file 2>/dev/null
sed -i '/^ \{0,\}#/d' $provider_file 2>/dev/null
sed -i 's/\t/ /g' $provider_file 2>/dev/null
provider_line=$(awk '{print $0"#*#"FNR}' $provider_file |grep -v '^ \{0,\}proxy-provider:\|^ \{0,\}Proxy:\|^ \{0,\}Proxy Group:\|^ \{0,\}Rule:\|^ \{0,\}type:\|^ \{0,\}path:\|^ \{0,\}url:\|^ \{0,\}interval:\|^ \{0,\}health-check:\|^ \{0,\}enable:' |awk -F '#*#' '{print $3}')
provider_num=$(grep -c "^ \{0,\}type:" $provider_file)
provider_count=1

cfg_get_dyn()
{
	echo "$(grep "^ \{0,\}$1" "$2" 2>/dev/null |grep -v "^ \{0,\}- name:"  |grep -v "^ \{0,\}- keep-alive" |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/\}.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)"
}

cfg_get()
{
	echo "$(grep "$1" "$2" 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null)"
}

cfg_gett()
{
	echo "$(grep "$1" $single_server 2>/dev/null |awk -v tag=$1 'BEGIN{FS=tag} {print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null |sed 's/ \{0,\}\}\{0,\}$//g' 2>/dev/null)"
}  

 if [ $loadservers -eq 1 ];then
#######READ SERVERS START   
if [ -f /tmp/yaml_proxy.yaml ];then
   while [[ "$( grep -c "config servers" $CFG_FILE )" -ne 0 ]] 
   do
      uci delete clash.@servers[0] && uci commit clash >/dev/null 2>&1
   done



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
   server_type="$(cfg_gett "type:")"
   #name
   server_name="$(cfg_gett "name:")"
   #server
   server="$(cfg_gett "server:")"
   #port
   port="$(cfg_gett "port:")"
   #cipher
   cipher="$(cfg_gett "cipher:")"
   #password
   password="$(cfg_gett "password:")"
   #protocol
   protocol="$(cfg_gett "protocol:")"
   #protocolparam
   protocolparam="$(cfg_gett "protocolparam:")"
   #obfsparam
   obfsparam="$(cfg_gett "obfsparam:")"
   #udp
   udp="$(cfg_gett "udp:")"
   #plugin:
   plugin="$(cfg_gett "plugin:")"
   #plugin-opts:
   plugin_opts="$(cfg_gett "plugin-opts:")"
   #obfs:
   obfs="$(cfg_gett "obfs:")"
   #obfs-host:
   obfs_host="$(cfg_gett "obfs-host:")"
   #psk:
   psk="$(cfg_gett "psk:")"
   #mode:
   mode="$(cfg_gett "mode:")"
   #tls:
   tls="$(cfg_gett "tls:")"
   #skip-cert-verify:
   verify="$(cfg_gett "skip-cert-verify:")"
   #mux:
   mux="$(cfg_gett "mux:")"
   #host:
   host="$(cfg_gett "host:")"
   #Host:
   Host="$(cfg_gett "Host:")"
   #path:
   path="$(cfg_gett "path:")"
   #ws-path:
   ws_path="$(cfg_gett "ws-path:")"
   #headers_custom:
   headers="$(cfg_gett "custom:")"
   #uuid:
   uuid="$(cfg_gett "uuid:")"
   #alterId:
   alterId="$(cfg_gett "alterId:")"
   #network
   network="$(cfg_gett "network:")"
   #username
   username="$(cfg_gett "username:")"
   #tls_custom:
   tls_custom="$(cfg_gett "tls:")"
   #sni:
   sni="$(cfg_gett "sni:")"
   #alpn:
   alpns="$(cfg_get_dyn "-" "$single_server")"
   #http_paths:
   http_paths="$(cfg_get_dyn "-" "$single_server")"   
   
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

	[ -z "$mode" ] && [ "$network" = "ws" ] && ${uci_set}obfs_vmess="websocket"
	   
	[ -z "$mode" ] && [ -z "$network" ] && ${uci_set}obfs_vmess="none"
	
	[ -z "$mode" ] && [ "$network" = "http" ] && ${uci_set}obfs_vmess="http"
	
   fi
   
   
    [ -z "$obfs_host" ] && ${uci_set}host="$host"

	[ -z "$mode" ] && [ "$server_type" = "snell" ] && ${uci_set}obfs_snell="$mode"
	
    [ -z "$obfs" ] && [ "$server_type" = "ss" ] && ${uci_set}obfs="$mode"
	
    [ -z "$obfs" ] && [ "$server_type" = "ss" ] && [ -z "$mode" ] && ${uci_set}obfs="none"
	
    [ -z "$mode" ] && [ "$server_type" = "snell" ] &&  ${uci_set}obfs_snell="none"

   if [ $tls ] && [ "$server_type" != "ss" ];then 
   ${uci_set}tls="$tls"
   fi
   ${uci_set}psk="$psk"
   if [ $verify ] && [ "$server_type" != "ssr" ];then
   ${uci_set}skip_cert_verify="$verify"
   fi

   ${uci_set}path="$path"
   [ -z "$path" ] && [ "$network" = "ws" ] && ${uci_set}path="$ws_path"
   ${uci_set}mux="$mux"
   ${uci_set}custom="$headers"
   
   [ -z "$headers" ] && [ "$network" = "ws" ] && ${uci_set}custom="$Host"
    
   if [ "$server_type" = "vmess" ]; then
    #v2ray
    ${uci_set}alterId="$alterId"
    ${uci_set}uuid="$uuid"
	${uci_del}http_path >/dev/null 2>&1
    for http_path in $http_paths; do
          ${uci_add}http_path="$http_path" >/dev/null 2>&1
    done
    if [ ! -z "$(grep "^ \{0,\}- keep-alive" "$single_server")" ]; then
          ${uci_set}keep_alive="true"
    else
          ${uci_set}keep_alive="false"
    fi
   fi
	
   if [ "$server_type" = "socks5" ] || [ "$server_type" = "http" ]; then
     ${uci_set}auth_name="$username"
     ${uci_set}auth_pass="$password"
   else
     ${uci_set}password="$password"
   fi
	if [ "$server_type" = "trojan" ]; then
       ${uci_set}sni="$sni"
       ${uci_del}alpn >/dev/null 2>&1
       for alpn in $alpns; do
        ${uci_add}alpn="$alpn" >/dev/null 2>&1
       done
	fi
	
done
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
fi
#######READ SERVERS END  
fi


if [ "${loadprovider}" -eq 1 ];then
#######READ PROVIDER START

if [ -f /tmp/yaml_provider.yaml ];then

while [[ "$( grep -c "config provider" $CFG_FILE )" -ne 0 ]] 
do
	uci delete clash.@provider[0] && uci commit clash 2>/dev/null	       
done
	
	

echo "" >"$match_provider"
provider_nums=0
config_load "clash"
config_foreach yml_provider_name_get "provider"


yml_provider_name_get()
{
   local section="$1"
   config_get "name" "$section" "name" ""
   [ ! -z "$name" ] && {
      echo "$provider_nums"."$name" >>"$match_provider"
   }
   provider_nums=$(( $provider_nums + 1 ))
}


	
cfg_new_provider_groups_get()
{
	 if [ -z "$1" ]; then
      return
   fi
   
   ${uci_add}groups="${1}"
}




for n in $provider_line
do
   [ "$provider_count" -eq 1 ] && {
      startLine="$n"
   }
   
   provider_count=$(expr "$provider_count" + 1)
   if [ "$provider_count" -gt "$provider_num" ]; then
      endLine=$(sed -n '$=' $provider_file)
   else
      endLine=$(expr $(echo "$provider_line" | sed -n "${provider_count}p") - 1)
   fi

   sed -n "${startLine},${endLine}p" $provider_file >$single_provider
   health_check_line=$(sed -n '/^ \{0,\}health-check:/=' $single_provider)
   sed -n "1,${health_check_line}p" $single_provider >$single_provider_gen
   sed -n "${health_check_line},\$p" $single_provider >$single_provider_che
   
   startLine=$(expr "$endLine" + 1)

   #name
   provider_name="$(sed -n "${n}p" $provider_file |awk -F ':' '{print $1}' |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)"
   
   #type
   provider_type="$(cfg_get "type:" "$single_provider_gen")"
   
   #path
   provider_path="$(cfg_get "path:" "$single_provider_gen")"

   #gen_url
   provider_gen_url="$(cfg_get "url:" "$single_provider_gen")"
   
   #gen_interval
   provider_gen_interval="$(cfg_get "interval:" "$single_provider_gen")"
   
   #che_enable
   provider_che_enable="$(cfg_get "enable:" "$single_provider_che")"
   
   #che_url
   provider_che_url="$(cfg_get "url:" "$single_provider_che")"
   
   #che_interval
   provider_che_interval="$(cfg_get "interval:" "$single_provider_che")"
   
  
    	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Now Reading 【$provider_type】-【$server_name】 Proxy-Provider..." >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			 echo "正在读取【$provider_type】-【$provider_name】代理集配置..." >$START_LOG
		fi

      name=clash
      uci_name_tmp=$(uci add $name provider)
      uci_set="uci -q set $name.$uci_name_tmp."
      uci_add="uci -q add_list $name.$uci_name_tmp."
      ${uci_set}name="$provider_name"
      ${uci_set}type="$provider_type"
      ${uci_set}path="$provider_path"
      ${uci_set}provider_url="$provider_gen_url"
      ${uci_set}provider_interval="$provider_gen_interval"
      ${uci_set}health_check="$provider_che_enable"
      ${uci_set}health_check_url="$provider_che_url"
      ${uci_set}health_check_interval="$provider_che_interval"


        if [ ! -z "$(grep "config groups" "$CFG_FILE")" ]; then
        for ((i=1;i<=$group_num;i++))
        do
            single_group="/tmp/group_$i.yaml"
            use_line=$(sed -n '/^ \{0,\}use:/=' $single_group)
            proxies_line=$(sed -n '/^ \{0,\}proxies:/=' $single_group)
            if [ "$use_line" -le "$proxies_line" ]; then
               if [ ! -z "$(sed -n "${use_line},${proxies_line}p" "$single_group" |grep -F "$provider_name")" ]; then
                  group_name=$(grep "name:" $single_group 2>/dev/null |awk -F 'name:' '{print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)
                  ${uci_add}groups="$group_name"
               fi
            elif [ "$use_line" -ge "$proxies_line" ]; then
               if [ ! -z "$(sed -n "${use_line},\$p" "$single_group" |grep -F "$provider_name")" ]; then
                  group_name=$(grep "name:" $single_group 2>/dev/null |awk -F 'name:' '{print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)
                  ${uci_add}groups="$group_name"
               fi
            elif [ ! -z "$use_line" ] && [ -z "$proxies_line" ]; then
         	    if [ ! -z "$(grep -F "$provider_name" $single_group)" ]; then
                  group_name=$(grep "name:" $single_group 2>/dev/null |awk -F 'name:' '{print $2}' 2>/dev/null |sed 's/,.*//' 2>/dev/null |sed 's/^ \{0,\}//g' 2>/dev/null |sed 's/ \{0,\}$//g' 2>/dev/null)
                  ${uci_add}groups="$group_name"
                fi
            fi
	    done
	    fi 

   uci commit clash
done


 	  	if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Reading Proxy Provider Completed" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
		elif [ $lang == "zh_cn" ];then
			echo "读取代理集配置完成" >$REAL_LOG
			sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG			
		fi
#######READ PROVIDER END
fi
fi


rm -rf /tmp/Proxy_Group /tmp/servers.yaml /tmp/yaml_proxy.yaml /tmp/group_*.yaml /tmp/yaml_group.yaml /tmp/match_servers.list /tmp/yaml_provider.yaml /tmp/provider.yaml /tmp/provider_gen.yaml /tmp/provider_che.yaml /tmp/match_provider.list 2>/dev/null

/usr/share/clash/proxy.sh >/dev/null 2>&1

}

run_load >/dev/null 2>&1

