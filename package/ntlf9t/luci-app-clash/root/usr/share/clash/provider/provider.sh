#!/bin/bash /etc/rc.common
. /lib/functions.sh


RULE_PROVIDER="/tmp/rule_provider.yaml"
GROUP_FILE="/tmp/groups.yaml"
CLASH_CONFIG="/etc/clash/config.yaml"
CONFIG_YAML_PATH=$(uci get clash.config.use_config 2>/dev/null)
if [  -f $CONFIG_YAML_PATH ] && [ "$(ls -l $CONFIG_YAML_PATH|awk '{print int($5)}')" -ne 0 ];then
	cp $CONFIG_YAML_PATH $CLASH_CONFIG 2>/dev/null		
fi
SCRIPT="/usr/share/clash/provider/script.yaml"
rule_providers=$(uci get clash.config.rule_providers 2>/dev/null)
CFG_FILE="/etc/config/clash"
config_name=$(uci get clash.config.name_tag 2>/dev/null)
lang=$(uci get luci.main.lang 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/custom/${config_name}.yaml"  
check_name=$(grep -F "${config_name}.yaml" "/usr/share/clashbackup/create_list.conf")
REAL_LOG="/usr/share/clash/clash_real.txt"
same_tag=$(uci get clash.config.same_tag 2>/dev/null)
rcount=$( grep -c "config ruleprovider" $CFG_FILE 2>/dev/null)
create=$(uci get clash.config.provider_config 2>/dev/null)
if [ "${create}" -eq 1 ];then

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




if [ -f $RULE_PROVIDER ];then
	rm -rf $RULE_PROVIDER 2>/dev/null
fi
	   
	   
rule_set()
{

   local section="$1"
   config_get "name" "$section" "name" ""
   config_get "type" "$section" "type" ""
   config_get "behavior" "$section" "behavior" ""
   config_get "path" "$section" "path" ""
   config_get "url" "$section" "url" ""
   config_get "interval" "$section" "interval" ""

   if [ "$path" != "./ruleprovider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./ruleprovider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
   
cat >> "$RULE_PROVIDER" <<-EOF
  $name:
    type: $type
    behavior: $behavior
    path: $path	
EOF

	

	

	if [ "$type" == "http" ]; then
cat >> "$RULE_PROVIDER" <<-EOF
    url: $url
    interval: $interval
EOF
	fi
}

if [ $rcount -gt 0 ];then	
	 config_load clash
	 config_foreach rule_set "ruleprovider"
fi

if [ -f $RULE_PROVIDER ];then 
	sed -i "1i\   " $RULE_PROVIDER 2>/dev/null 
	sed -i "2i\rule-providers:" $RULE_PROVIDER 2>/dev/null
fi


PROVIDER_FILE="/tmp/yaml_provider.yaml"
pcount=$( grep -c "config proxyprovider" $CFG_FILE 2>/dev/null)

if [ -f $PROVIDER_FILE ];then
	rm -rf $PROVIDER_FILE 2>/dev/null
fi

yml_proxy_provider_set()
{
   local section="$1"

   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "path" "$section" "path" ""
   config_get "provider_url" "$section" "provider_url" ""
   config_get "provider_interval" "$section" "provider_interval" ""
   config_get "health_check" "$section" "health_check" ""
   config_get "health_check_url" "$section" "health_check_url" ""
   config_get "health_check_interval" "$section" "health_check_interval" ""
   
   if [ "$path" != "./proxyprovider/$name.yaml" ] && [ "$type" = "http" ]; then
      path="./proxyprovider/$name.yaml"
   elif [ -z "$path" ]; then
      return
   fi
	  

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi

   
   if [ -z "$health_check" ]; then
      return
   fi

   
   echo "$name" >> /tmp/Proxy_Provider
   
cat >> "$PROVIDER_FILE" <<-EOF
  $name:
    type: $type
    path: $path
EOF
   if [ ! -z "$provider_url" ]; then
cat >> "$PROVIDER_FILE" <<-EOF
    url: $provider_url
    interval: $provider_interval
EOF
   fi
cat >> "$PROVIDER_FILE" <<-EOF
    health-check:
      enable: $health_check
      url: $health_check_url
      interval: $health_check_interval
EOF

}


if [ $pcount -gt 0 ];then
	config_load "clash"
	config_foreach yml_proxy_provider_set "proxyprovider"
fi

if [ -f $PROVIDER_FILE ];then 
	sed -i "1i\   " $PROVIDER_FILE 2>/dev/null 
	sed -i "2i\proxy-providers:" $PROVIDER_FILE 2>/dev/null
	rm -rf /tmp/Proxy_Provider
fi


if [ -f $GROUP_FILE ];then
	rm -rf $GROUP_FILE 2>/dev/null
fi

set_groups()
{
  if [ -z "$1" ]; then
     return
  fi

	if [ "$1" = "$3" ]; then
	   set_group=1
	   echo "  - \"${2}\"" >>$GROUP_FILE
	fi

}


set_other_groups()
{
   set_group=1
   if [ "${1}" = "DIRECT" ]||[ "${1}" = "REJECT" ];then
   echo "    - ${1}" >>$GROUP_FILE 2>/dev/null 
   else
   echo "    - \"${1}\"" >>$GROUP_FILE 2>/dev/null 
   fi

}

set_proxy_provider()
{
	local section="$1"
	config_get "name" "$section" "name" ""
	config_list_foreach "$section" "pgroups" set_provider_groups "$name" "$2"

}

set_provider_groups()
{
	if [ -z "$1" ]; then
		return
	fi

	if [ "$1" = "$3" ]; then
	   set_proxy_provider=1
	   echo "    - ${2}" >>$GROUP_FILE
	fi

}

yml_groups_set()
{

   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "old_name" "$section" "old_name" ""
   config_get "test_url" "$section" "test_url" ""
   config_get "test_interval" "$section" "test_interval" ""
   config_get "other_group" "$section" "other_group" ""

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   echo "- name: $name" >>$GROUP_FILE 2>/dev/null 
   echo "  type: $type" >>$GROUP_FILE 2>/dev/null 
   group_name="$name"
   echo "  proxies: " >>$GROUP_FILE
   
   set_group=0
   set_proxy_provider=0 
   
   
  	
   config_list_foreach "$section" "other_group" set_other_groups

   if [ "$( grep -c "config proxyprovider" $CFG_FILE )" -gt 0 ];then  

		    echo "  use: $group_name" >>$GROUP_FILE	   
		    if [ "$type" != "relay" ]; then
				 config_foreach set_proxy_provider "proxyprovider" "$group_name" 
		    fi

		    if [ "$set_proxy_provider" -eq 1 ]; then
			  sed -i "/^ \{0,\}use: ${group_name}/c\  use:" $GROUP_FILE
		    else
			  sed -i "/use: ${group_name}/d" $GROUP_FILE 
		    fi
			

		    if [ "$set_group" -eq 1 ]; then
			  sed -i "/^ \{0,\}proxies: ${group_name}/c\  proxies:" $GROUP_FILE
		    else
			  sed -i "/proxies: ${group_name}/d" $GROUP_FILE 
		    fi	     
   fi      
 
   
    [ ! -z "$test_url" ] && {
		echo "  url: $test_url" >>$GROUP_FILE 2>/dev/null 
    }
    [ ! -z "$test_interval" ] && {
		echo "  interval: \"$test_interval\"" >>$GROUP_FILE 2>/dev/null 
    }
}

gcount=$( grep -c "config pgroups" $CFG_FILE 2>/dev/null)
if [ $gcount -gt 0 ];then
	config_load clash
	config_foreach yml_groups_set "pgroups"
fi


if [ -f $GROUP_FILE ]; then
	sed -i "1i\  " $GROUP_FILE 2>/dev/null 
	sed -i "2i\proxy-groups:" $GROUP_FILE 2>/dev/null 
fi


RULE_FILE="/tmp/rules.yaml"
rucount=$( grep -c "config rules" $CFG_FILE 2>/dev/null)

if [ -f $RULE_FILE ];then
	rm -rf $RULE_FILE 2>/dev/null
fi
	   
	   
add_rules()
{
	   local section="$1"
	   config_get "rulegroups" "$section" "rulegroups" ""
	   config_get "rulename" "$section" "rulename" ""
	   config_get "type" "$section" "type" ""
	   config_get "res" "$section" "res" ""
	   config_get "rulenamee" "$section" "rulenamee" ""
	   
	    if [ ! -z $rulename ];then
	      rulename=$rulename
		elif [ ! -z $rulenamee ];then
		  rulename=$rulenamee
		fi	
		  
	   if [ "${res}" -eq 1 ];then
		echo "- $type,$rulename,$rulegroups,no-resolve">>$RULE_FILE
	   elif [ "${type}" == "MATCH" ];then
	    echo "- $type,$rulegroups">>$RULE_FILE
	   else
		echo "- $type,$rulename,$rulegroups">>$RULE_FILE
	   fi
}

if [ $rucount -gt 0 ];then	
 config_load clash
 config_foreach add_rules "rules"
fi
 
if [ -f $RULE_FILE ];then 
	sed -i "1i\   " $RULE_FILE 2>/dev/null 
	sed -i "2i\rules:" $RULE_FILE 2>/dev/null
fi 

mode=$(uci get clash.config.mode 2>/dev/null)
p_mode=$(uci get clash.config.p_mode 2>/dev/null)
da_password=$(uci get clash.config.dash_pass 2>/dev/null)
redir_port=$(uci get clash.config.redir_port 2>/dev/null)
http_port=$(uci get clash.config.http_port 2>/dev/null)
socks_port=$(uci get clash.config.socks_port 2>/dev/null)
dash_port=$(uci get clash.config.dash_port 2>/dev/null)
bind_addr=$(uci get clash.config.bind_addr 2>/dev/null)
allow_lan=$(uci get clash.config.allow_lan 2>/dev/null)
log_level=$(uci get clash.config.level 2>/dev/null)
subtype=$(uci get clash.config.subcri 2>/dev/null)
DNS_FILE="/usr/share/clash/dns.yaml" 
TEMP_FILE="/tmp/dns_temp.yaml"

cat >> "$TEMP_FILE" <<-EOF
#config-start-here
EOF

		sed -i "1i\port: ${http_port}" $TEMP_FILE 2>/dev/null
		sed -i "/port: ${http_port}/a\socks-port: ${socks_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/socks-port: ${socks_port}/a\redir-port: ${redir_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/redir-port: ${redir_port}/a\allow-lan: ${allow_lan}" $TEMP_FILE 2>/dev/null 
		if [ $allow_lan == "true" ];  then
		sed -i "/allow-lan: ${allow_lan}/a\bind-address: \"${bind_addr}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/bind-address: \"${bind_addr}\"/a\mode: ${p_mode}" $TEMP_FILE 2>/dev/null
		sed -i "/mode: ${p_mode}/a\log-level: ${log_level}" $TEMP_FILE 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $TEMP_FILE 2>/dev/null 
		sed -i "external-ui: \"/usr/share/clash/dashboard\"/a\  " $TEMP_FILE 2>/dev/null 
		sed -i "   /a\   " $TEMP_FILE 2>/dev/null
		else
		sed -i "/allow-lan: ${allow_lan}/a\mode: ${p_mode}" $TEMP_FILE 2>/dev/null
		sed -i "/mode: ${p_mode}/a\log-level: ${log_level}" $TEMP_FILE 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $TEMP_FILE 2>/dev/null 
		fi
		sed -i '/#config-start-here/ d' $TEMP_FILE 2>/dev/null

		
cat $DNS_FILE >> $TEMP_FILE  2>/dev/null


script=$(uci get clash.config.script 2>/dev/null)
ruleprovider=$(uci get clash.config.rulprp 2>/dev/null)
ppro=$(uci get clash.config.ppro 2>/dev/null)
rul=$(uci get clash.config.rul 2>/dev/null)

if [ $ppro -eq 1 ];then
if [ -f $PROVIDER_FILE ];then 
cat $PROVIDER_FILE >> $TEMP_FILE 2>/dev/null
fi
fi

if [ -f $GROUP_FILE ];then
cat $GROUP_FILE >> $TEMP_FILE 2>/dev/null
fi


if [ $ruleprovider -eq 1 ];then
if [ -f $RULE_PROVIDER ];then
cat $RULE_PROVIDER >> $TEMP_FILE  2>/dev/null
sed -i -e '$a\' $TEMP_FILE  2>/dev/null
fi
fi

if [ $script -eq 1 ];then
if [ -f $SCRIPT ];then
cat $SCRIPT >> $TEMP_FILE  2>/dev/null
sed -i -e '$a\' $TEMP_FILE  2>/dev/null
fi
fi


if [ $rul -eq 1 ];then
if [ -f $RULE_FILE ];then
cat $RULE_FILE >> $TEMP_FILE 2>/dev/null
fi
fi

mv $TEMP_FILE  $CONFIG_YAML 2>/dev/null

if [ -z $check_name ] && [ "${same_tag}" -eq 1 ];then
echo "${config_name}.yaml" >>/usr/share/clashbackup/create_list.conf
elif [ -z $check_name ] && [ "${same_tag}" -eq 0 ];then
echo "${config_name}.yaml" >>/usr/share/clashbackup/create_list.conf
fi

rm -rf $RULE_PROVIDER $PROVIDER_FILE $GROUP_FILE  $RULE_FILE

if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Completed Creating Custom Config.. " >$REAL_LOG 
		 sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
elif [ $lang == "zh_cn" ];then
    	echo "创建自定义配置完成..." >$REAL_LOG
		sleep 2
		echo "Clash for OpenWRT" >$REAL_LOG
fi
fi
fi

	