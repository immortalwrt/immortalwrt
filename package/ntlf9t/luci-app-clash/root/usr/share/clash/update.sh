#!/bin/bash /etc/rc.common
. /lib/functions.sh  

config_name=$(uci get clash.config.config_update_name 2>/dev/null)
CONFIG_YAML="/usr/share/clash/config/sub/${config_name}" 
url=$(grep -F "${config_name}" "/usr/share/clashbackup/confit_list.conf" | awk -F '#' '{print $2}')
lang=$(uci get luci.main.lang 2>/dev/null)
REAL_LOG="/usr/share/clash/clash_real.txt"
c_type=$(uci get clash.config.config_type 2>/dev/null)
path=$(uci get clash.config.use_config 2>/dev/null)
type=$(grep -F "${config_name}" "/usr/share/clashbackup/confit_list.conf" | awk -F '#' '{print $3}') 

if [ $type == "clash" ] && [ ! -z $url ];then

	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始更新配置" >$REAL_LOG
	fi
	wget --no-check-certificate --user-agent="Clash/OpenWRT" $url -O 2>&1 >1 $CONFIG_YAML
	
	if [ "$?" -eq "0" ]; then
		if [ $lang == "en" ] || [ $lang == "auto" ];then
			echo "Updating Configuration Completed" >$REAL_LOG

		elif [ $lang == "zh_cn" ];then
			echo "更新配置完成" >$REAL_LOG

		fi
		sleep 5
		echo "Clash for OpenWRT" >$REAL_LOG
		
			
	fi
fi

	
if [ $type == "v2ssr2clash" ] && [ ! -z "${url}" ];then


	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Updating Configuration..." >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
				echo "开始更新配置" >$REAL_LOG
	fi
	
	#awk '/config groups/,/##end/{print}' /etc/config/clashh 2>/dev/null >/usr/share/clash/v2ssr/config.bak 2>&1

	
	CFG_FILE="/etc/config/clash"
	
    while [[ "$( grep -c "config groups" $CFG_FILE )" -ne 0 ]] 
    do
      uci delete clash.@groups[0] && uci commit clash >/dev/null 2>&1
    done
   
    while [[ "$( grep -c "config servers" $CFG_FILE )" -ne 0 ]] 
    do
      uci delete clash.@servers[0] && uci commit clash >/dev/null 2>&1
    done  

	cat /etc/config/clash  /usr/share/clash/v2ssr/policygroup >/usr/share/clash/v2ssr/clash 2>/dev/null
	
	mv /usr/share/clash/v2ssr/clash /etc/config/clash 2>/dev/null
	
	
# functions for parsing and generating json

_json_get_var() {
	# dest=$1
	# var=$2
	eval "$1=\"\$${JSON_PREFIX}$2\""
}

_json_set_var() {
	# var=$1
	local ___val="$2"
	eval "${JSON_PREFIX}$1=\"\$___val\""
}

__jshn_raw_append() {
	# var=$1
	local value="$2"
	local sep="${3:- }"

	eval "export -- \"$1=\${$1:+\${$1}\${value:+\$sep}}\$value\""
}

_jshn_append() {
	# var=$1
	local _a_value="$2"
	eval "${JSON_PREFIX}$1=\"\${${JSON_PREFIX}$1} \$_a_value\""
}

_get_var() {
	# var=$1
	# value=$2
	eval "$1=\"\$$2\""
}

_set_var() {
	# var=$1
	local __val="$2"
	eval "$1=\"\$__val\""
}

_json_inc() {
	# var=$1
	# dest=$2

	let "${JSON_PREFIX}$1 += 1" "$2 = ${JSON_PREFIX}$1"
}

_json_add_generic() {
	# type=$1
	# name=$2
	# value=$3
	# cur=$4

	local var
	if [ "${4%%[0-9]*}" = "J_A" ]; then
		_json_inc "S_$4" var
	else
		var="${2//[^a-zA-Z0-9_]/_}"
		[[ "$var" == "$2" ]] || export -- "${JSON_PREFIX}N_${4}_${var}=$2"
	fi

	export -- \
		"${JSON_PREFIX}${4}_$var=$3" \
		"${JSON_PREFIX}T_${4}_$var=$1"
	_jshn_append "JSON_UNSET" "${4}_$var"
	_jshn_append "K_$4" "$var"
}

_json_add_table() {
	# name=$1
	# type=$2
	# itype=$3
	local cur seq

	_json_get_var cur JSON_CUR
	_json_inc JSON_SEQ seq

	local table="J_$3$seq"
	_json_set_var "U_$table" "$cur"
	export -- "${JSON_PREFIX}K_$table="
	unset "${JSON_PREFIX}S_$table"
	_json_set_var JSON_CUR "$table"
	_jshn_append "JSON_UNSET" "$table"

	_json_add_generic "$2" "$1" "$table" "$cur"
}

_json_close_table() {
	local _s_cur

	_json_get_var _s_cur JSON_CUR
	_json_get_var "${JSON_PREFIX}JSON_CUR" "U_$_s_cur"
}

json_set_namespace() {
	local _new="$1"
	local _old="$2"

	[ -n "$_old" ] && _set_var "$_old" "$JSON_PREFIX"
	JSON_PREFIX="$_new"
}

json_cleanup() {
	local unset tmp

	_json_get_var unset JSON_UNSET
	for tmp in $unset J_V; do
		unset \
			${JSON_PREFIX}U_$tmp \
			${JSON_PREFIX}K_$tmp \
			${JSON_PREFIX}S_$tmp \
			${JSON_PREFIX}T_$tmp \
			${JSON_PREFIX}N_$tmp \
			${JSON_PREFIX}$tmp
	done

	unset \
		${JSON_PREFIX}JSON_SEQ \
		${JSON_PREFIX}JSON_CUR \
		${JSON_PREFIX}JSON_UNSET
}

json_init() {
	json_cleanup
	export -n ${JSON_PREFIX}JSON_SEQ=0
	export -- \
		${JSON_PREFIX}JSON_CUR="J_V" \
		${JSON_PREFIX}K_J_V=
}

json_add_object() {
	_json_add_table "$1" object T
}

json_close_object() {
	_json_close_table
}

json_add_array() {
	_json_add_table "$1" array A
}

json_close_array() {
	_json_close_table
}

json_add_string() {
	local cur
	_json_get_var cur JSON_CUR
	_json_add_generic string "$1" "$2" "$cur"
}

json_add_int() {
	local cur
	_json_get_var cur JSON_CUR
	_json_add_generic int "$1" "$2" "$cur"
}

json_add_boolean() {
	local cur
	_json_get_var cur JSON_CUR
	_json_add_generic boolean "$1" "$2" "$cur"
}

json_add_double() {
	local cur
	_json_get_var cur JSON_CUR
	_json_add_generic double "$1" "$2" "$cur"
}

json_add_null() {
	local cur
	_json_get_var cur JSON_CUR
	_json_add_generic null "$1" "" "$cur"
}

# functions read access to json variables

json_load() {
	eval "`jshn -r "$1"`"
}

json_load_file() {
	eval "`jshn -R "$1"`"
}

json_dump() {
	jshn "$@" ${JSON_PREFIX:+-p "$JSON_PREFIX"} -w 
}

json_get_type() {
	local __dest="$1"
	local __cur

	_json_get_var __cur JSON_CUR
	local __var="${JSON_PREFIX}T_${__cur}_${2//[^a-zA-Z0-9_]/_}"
	eval "export -- \"$__dest=\${$__var}\"; [ -n \"\${$__var+x}\" ]"
}

json_get_keys() {
	local __dest="$1"
	local _tbl_cur

	if [ -n "$2" ]; then
		json_get_var _tbl_cur "$2"
	else
		_json_get_var _tbl_cur JSON_CUR
	fi
	local __var="${JSON_PREFIX}K_${_tbl_cur}"
	eval "export -- \"$__dest=\${$__var}\"; [ -n \"\${$__var+x}\" ]"
}

json_get_values() {
	local _v_dest="$1"
	local _v_keys _v_val _select=
	local _json_no_warning=1

	unset "$_v_dest"
	[ -n "$2" ] && {
		json_select "$2" || return 1
		_select=1
	}

	json_get_keys _v_keys
	set -- $_v_keys
	while [ "$#" -gt 0 ]; do
		json_get_var _v_val "$1"
		__jshn_raw_append "$_v_dest" "$_v_val"
		shift
	done
	[ -n "$_select" ] && json_select ..

	return 0
}

json_get_var() {
	local __dest="$1"
	local __cur

	_json_get_var __cur JSON_CUR
	local __var="${JSON_PREFIX}${__cur}_${2//[^a-zA-Z0-9_]/_}"
	eval "export -- \"$__dest=\${$__var:-$3}\"; [ -n \"\${$__var+x}\${3+x}\" ]"
}

json_get_vars() {
	while [ "$#" -gt 0 ]; do
		local _var="$1"; shift
		if [ "$_var" != "${_var#*:}" ]; then
			json_get_var "${_var%%:*}" "${_var%%:*}" "${_var#*:}"
		else
			json_get_var "$_var" "$_var"
		fi
	done
}

json_select() {
	local target="$1"
	local type
	local cur

	[ -z "$1" ] && {
		_json_set_var JSON_CUR "J_V"
		return 0
	}
	[[ "$1" == ".." ]] && {
		_json_get_var cur JSON_CUR
		_json_get_var cur "U_$cur"
		_json_set_var JSON_CUR "$cur"
		return 0
	}
	json_get_type type "$target"
	case "$type" in
		object|array)
			json_get_var cur "$target"
			_json_set_var JSON_CUR "$cur"
		;;
		*)
			[ -n "$_json_no_warning" ] || \
				echo "WARNING: Variable '$target' does not exist or is not an array/object"
			return 1
		;;
	esac
}

json_is_a() {
	local type

	json_get_type type "$1"
	[ "$type" = "$2" ]
}

json_for_each_item() {
	[ "$#" -ge 2 ] || return 0
	local function="$1"; shift
	local target="$1"; shift
	local type val

	json_get_type type "$target"
	case "$type" in
		object|array)
			local keys key
			json_select "$target"
			json_get_keys keys
			for key in $keys; do
				json_get_var val "$key"
				eval "$function \"\$val\" \"\$key\" \"\$@\""
			done
			json_select ..
		;;
		*)
			json_get_var val "$target"
			eval "$function \"\$val\" \"\" \"\$@\""
		;;
	esac
}

REAL_LOG="/usr/share/clash/clash_real.txt"
lang=$(uci get luci.main.lang 2>/dev/null)

urlsafe_b64decode() {
    local d="====" data=$(echo $1 | sed 's/_/\//g; s/-/+/g')
    local mod4=$((${#data}%4))
    [ $mod4 -gt 0 ] && data=${data}${d:mod4}
    echo $data | base64 -d
}



Server_Update() {
    local uci_set="uci -q set $name.$1."
    ${uci_set}name="$ssr_remarks"
    ${uci_set}type="$ssr_type"
    ${uci_set}server="$ssr_host"
    ${uci_set}port="$ssr_port"
    uci -q get $name.@servers[$1].timeout >/dev/null || ${uci_set}timeout="60"
    ${uci_set}password="$ssr_passwd"
    ${uci_set}cipher_ssr="$ssr_method"
    ${uci_set}protocol="$ssr_protocol"
    ${uci_set}protocolparam="$ssr_protoparam"
    ${uci_set}obfs_ssr="$ssr_obfs"
    ${uci_set}obfsparam="$ssr_obfsparam"

    
	if [ "$ssr_type" = "vmess" ]; then
    #v2ray
    ${uci_set}alterId="$ssr_alter_id"
    ${uci_set}uuid="$ssr_vmess_id"
	if [ "$ssr_security" = "none" ];then
    ${uci_set}securitys="auto"
	else
	${uci_set}securitys="$ssr_security"
	fi
	if [ "$ssr_transport" = "tcp" ];then
    ${uci_set}obfs_vmess="none"
	else
	${uci_set}obfs_vmess="websocket"
	fi
    ${uci_set}custom_host="$ssr_ws_host"
    ${uci_set}path="$ssr_ws_path"
    ${uci_set}tls="$ssr_tls"
	fi
}



echo "1" >/www/lock.htm

name=clash
subscribe_url=$url
[ ${#subscribe_url[@]} -eq 0 ] && exit 1

for ((o=0;o<${#subscribe_url[@]};o++))
do
		  	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration..." >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "正在下载配置..." >$REAL_LOG
			fi
			sleep 2
	subscribe_data=$(wget --user-agent="User-Agent: Mozilla" --no-check-certificate -T 3 -O- ${subscribe_url[o]})
	curl_code=$?
	if [ ! $curl_code -eq 0 ];then

		subscribe_data=$(wget --no-check-certificate -T 3 -O- ${subscribe_url[o]})
		curl_code=$?
	fi
	
			if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Downloading Configuration Completed" >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "下载配置完成" >$REAL_LOG
			fi
			sleep 2
			
 	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Strating to Create Custom Config.. " >$REAL_LOG 
	elif [ $lang == "zh_cn" ];then
    	 echo "开始创建自定义配置..." >$REAL_LOG
	fi
	sleep 2
	
	if [ $curl_code -eq 0 ];then
		ssr_url=($(echo $subscribe_data | base64 -d | sed 's/\r//g')) 
		subscribe_max=$(echo ${ssr_url[0]} | grep -i MAX= | awk -F = '{print $2}')
		subscribe_max_x=()
			if [ -n "$subscribe_max" ]; then
				while [ ${#subscribe_max_x[@]} -ne $subscribe_max ]
				do
					if [ ${#ssr_url[@]} -ge 10 ]; then
						if [ $((${RANDOM:0:2}%2)) -eq 0 ]; then
							temp_x=${RANDOM:0:1}
						else
							temp_x=${RANDOM:0:2}
						fi
					else
						temp_x=${RANDOM:0:1}
					fi
					[ $temp_x -lt ${#ssr_url[@]} -a -z "$(echo "${subscribe_max_x[*]}" | grep -w $temp_x)" ] && subscribe_max_x[${#subscribe_max_x[@]}]="$temp_x"
				done
			else
				subscribe_max=${#ssr_url[@]}
			fi
			
			ssr_group=$(urlsafe_b64decode $(urlsafe_b64decode ${ssr_url[$((${#ssr_url[@]} - 1))]//ssr:\/\//} | sed 's/&/\n/g' | grep group= | awk -F = '{print $2}'))
			if [ -z "$ssr_group" ]; then
				ssr_group="default"
			fi
			if [ -n "$ssr_group" ]; then
				subscribe_i=0
				subscribe_n=0
				subscribe_o=0
				subscribe_x=""
				temp_host_o=()
					curr_ssr=$(uci show $name | grep @servers | grep -c server=)
					for ((x=0;x<$curr_ssr;x++)) 
					do
						temp_alias=$(uci -q get $name.@servers[$x].grouphashkey | grep "$ssr_grouphashkey")
						[ -n "$temp_alias" ] && temp_host_o[${#temp_host_o[@]}]=$(uci get $name.@servers[$x].hashkey)
					done

					for ((x=0;x<$subscribe_max;x++))
					do
						[ ${#subscribe_max_x[@]} -eq 0 ] && temp_x=$x || temp_x=${subscribe_max_x[x]}
						result=$(echo ${ssr_url[temp_x]} | grep "ssr")
						if [[ "$result" != "" ]]
						then
							temp_info=$(urlsafe_b64decode ${ssr_url[temp_x]//ssr:\/\//}) 
							
							ssr_hashkey=$(echo "$temp_info" | md5sum | cut -d ' ' -f1)


							info=${temp_info///?*/}
							temp_info_array=(${info//:/ })
							ssr_type="ssr"
							ssr_host=${temp_info_array[0]}
							ssr_port=${temp_info_array[1]}
							ssr_protocol=${temp_info_array[2]}
							ssr_method=${temp_info_array[3]}
							ssr_obfs=${temp_info_array[4]}
							ssr_passwd=$(urlsafe_b64decode ${temp_info_array[5]})
							info=${temp_info:$((${#info} + 2))}
							info=(${info//&/ })
							ssr_protoparam=""
							ssr_obfsparam=""
							ssr_remarks="$temp_x"
							for ((i=0;i<${#info[@]};i++)) 
							do
								temp_info=($(echo ${info[i]} | sed 's/=/ /g'))
								case "${temp_info[0]}" in
								protoparam)
								ssr_protoparam=$(urlsafe_b64decode ${temp_info[1]})
							;;
							obfsparam)
							ssr_obfsparam=$(urlsafe_b64decode ${temp_info[1]})
						;;
						remarks)
						ssr_remarks=$(urlsafe_b64decode ${temp_info[1]})
					;;
					esac
				done
			else
				temp_info=$(urlsafe_b64decode ${ssr_url[temp_x]//vmess:\/\//}) 
				
				ssr_hashkey=$(echo "$temp_info" | md5sum | cut -d ' ' -f1)

				ssr_type="vmess"
				json_load "$temp_info"
				json_get_var ssr_host add
				json_get_var ssr_port port
				json_get_var ssr_alter_id aid
				json_get_var ssr_vmess_id id
				json_get_var ssr_security type
				json_get_var ssr_transport net
				json_get_var ssr_remarks ps				
				json_get_var ssr_ws_host host
				json_get_var ssr_ws_path path
				json_get_var ssr_tls tls
				if [ "$ssr_tls" == "tls" -o "$ssr_tls" == "1" ]; then
					ssr_tls="true"
				else
				    ssr_tls="false"
				fi
			fi

			if [ -z "ssr_remarks" ]; then 
				ssr_remarks="$ssr_host:$ssr_port";
			fi

			uci_name_tmp=$(uci show $name | grep -w "$ssr_hashkey" | awk -F . '{print $2}')
			if [ -z "$uci_name_tmp" ]; then 
				uci_name_tmp=$(uci add $name servers)
				subscribe_n=$(($subscribe_n + 1))
			fi

		  	if [ $lang == "en" ] || [ $lang == "auto" ];then
				echo "Decoding 【$ssr_type】-【$ssr_remarks】 Proxy..." >$REAL_LOG
			elif [ $lang == "zh_cn" ];then
				echo "正在解码 【$ssr_type】-【$ssr_remarks】 代理..." >$REAL_LOG
			fi
			
			
			Server_Update $uci_name_tmp
			subscribe_x=$subscribe_x$ssr_hashkey" "
			ssrtype=$(echo $ssr_type | tr '[a-z]' '[A-Z]')
			
			
		done

		for ((x=0;x<${#temp_host_o[@]};x++)) 
		do
			if [ -z "$(echo "$subscribe_x" | grep -w ${temp_host_o[x]})" ]; then
				uci_name_tmp=$(uci show $name | grep ${temp_host_o[x]} | awk -F . '{print $2}')
				uci delete $name.$uci_name_tmp
				subscribe_o=$(($subscribe_o + 1))
			fi
		done
		uci commit $name
	
	fi
fi
done
echo "0" >/www/lock.htm

. /lib/functions.sh



config_type=$(uci get clash.config.config_type 2>/dev/null)



	
CONFIG_YAML_RULE="/usr/share/clash/v2ssr/v2ssr_custom_rule.yaml"
SERVER_FILE="/tmp/servers.yaml"
CONFIG_YAML="/usr/share/clash/config/sub/${config_name}"
TEMP_FILE="/tmp/dns_temp.yaml"
Proxy_Group="/tmp/Proxy_Group"
GROUP_FILE="/tmp/groups.yaml"
CONFIG_FILE="/tmp/y_groups"
DNS_FILE="/usr/share/clash/dns.yaml" 


   servcount=$( grep -c "config servers" $CFG_FILE 2>/dev/null)
   gcount=$( grep -c "config groups" $CFG_FILE 2>/dev/null)
   if [ $servcount -eq 0 ] || [ $gcount -eq 0 ];then
 	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "No servers or group. Aborting Operation .." >$REAL_LOG 
		sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
    	 echo "找不到代理或策略组。中止操作..." >$REAL_LOG
		 sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	fi
	exit 0	
   fi
   sleep 2
servers_set()
{
   local section="$1"
   config_get "type" "$section" "type" ""
   config_get "name" "$section" "name" ""
   config_get "server" "$section" "server" ""
   config_get "port" "$section" "port" ""
   config_get "cipher" "$section" "cipher" ""
   config_get "password" "$section" "password" ""
   config_get "securitys" "$section" "securitys" ""
   config_get "udp" "$section" "udp" ""
   config_get "obfs" "$section" "obfs" ""
   config_get "obfs_vmess" "$section" "obfs_vmess" ""
   config_get "host" "$section" "host" ""
   config_get "custom" "$section" "custom" ""
   config_get "tls" "$section" "tls" ""
   config_get "tls_custom" "$section" "tls_custom" ""
   config_get "skip_cert_verify" "$section" "skip_cert_verify" ""
   config_get "path" "$section" "path" ""
   config_get "alterId" "$section" "alterId" ""
   config_get "uuid" "$section" "uuid" ""
   config_get "auth_name" "$section" "auth_name" ""
   config_get "auth_pass" "$section" "auth_pass" ""
   config_get "mux" "$section" "mux" ""
   config_get "protocol" "$section" "protocol" ""
   config_get "protocolparam" "$section" "protocolparam" ""
   config_get "obfsparam" "$section" "obfsparam" ""
   config_get "obfs_ssr" "$section" "obfs_ssr" ""
   config_get "cipher_ssr" "$section" "cipher_ssr" ""
   config_get "psk" "$section" "psk" ""
   config_get "obfs_snell" "$section" "obfs_snell" ""
	
   if [ -z "$type" ]; then
      return
   fi
   
	if [ ! -z "$protocolparam" ];then
	  pro_param=", protocolparam: $protocolparam"	
	else
	  pro_param=", protocolparam: ''" 
	fi

	if [ ! -z "$protocol" ] && [ "$type" = "ssr" ];then
	  protol=", protocol: $protocol"
	else
	  protol=", protocol: origin"	 
	fi
	
	if [ ! -z "$obfs_ssr" ];then
	 ssr_obfs=", obfs: $obfs_ssr"
	else
	 ssr_obfs=", obfs: plain"
	fi
	
	if [ ! -z "$obfsparam" ];then
	 obfs_param=", obfsparam: $obfsparam"
         else
	obfs_param=", obfsparam: ''"
	fi 
   
   if [ -z "$server" ]; then
      return
   fi

   if [ ! -z "$mux" ]; then
      muxx="mux: $mux"
   fi
   if [ "$obfs_snell" = "none" ]; then
      obfs_snell=""
   fi
   
   if [ -z "$name" ]; then
      name="Server"
   fi
   
   if [ -z "$port" ]; then
      return
   fi
   
   if [ ! -z "$udp" ] && [ "$obfs" ] || [ "$obfs" = " " ]; then
      udpp=", udp: $udp"
   fi
   
   if [ "$obfs" != "none" ] && [ "$type" = "ss" ]; then
      if [ "$obfs" = "websocket" ]; then
         obfss="plugin: v2ray-plugin"
      else
         obfss="plugin: obfs"
      fi
   fi
   
   if [ "$obfs_vmess" = "none" ] && [ "$type" = "vmess" ]; then
      	obfs_vmesss=""
   elif [ "$obfs_vmess" != "none" ] && [ "$type" = "vmess" ]; then 
      	obfs_vmesss=", network: ws"
   fi  
   

   
   if [ ! -z "$custom" ] && [ "$type" = "vmess" ]; then
      custom=", ws-headers: { Host: $custom }"
   fi
   
   if [ ! "$tls" ] && [ "$type" = "vmess" ]; then
       tlss=""
   elif [ "$tls" ] && [ "$type" = "vmess" ]; then
      tlss=", tls: $tls"
   elif [ "$tls" ] && [ "$type" = "http" ]; then
	  tls_hs=", tls: $tls" 
   elif [ "$tls" ] && [ "$type" = "socks5" ]; then
	  tls_hs=", tls: $tls"	  
   fi
   
   if [ ! -z "$path" ]; then
      if [ "$type" != "vmess" ]; then
         paths="path: '$path'"
      else
         path=", ws-path: $path"
      fi
   fi

   if [ "$skip_cert_verify" = "true" ] && [ "$type" != "ss" ]; then
      skip_cert_verifys=", skip-cert-verify: $skip_cert_verify"
  elif [ ! "$skip_cert_verify" ]; then
      skip_cert_verifys=""	  
   fi

   
   
   if [ "$type" = "vmess" ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, uuid: $uuid, alterId: $alterId, cipher: $securitys$obfs_vmesss$path$custom$tlss$skip_cert_verifys }" >>$SERVER_FILE
   fi
   

    if [ "$type" = "ssr" ]; then
      echo "- { name: \"$name\", type: $type, server: $server, port: $port, cipher: $cipher_ssr, password: "$password"$protol$pro_param$ssr_obfs$obfs_param}" >>$SERVER_FILE
    fi



}

config_load clash
config_foreach servers_set "servers"

if [ "$(ls -l $SERVER_FILE|awk '{print $5}')" -ne 0 ]; then

sed -i "1i\   " $SERVER_FILE 2>/dev/null 
sed -i "2i\Proxy:" $SERVER_FILE 2>/dev/null 

egrep '^ {0,}-' $SERVER_FILE |grep name: |awk -F 'name: ' '{print $2}' |sed 's/,.*//' >$Proxy_Group 2>&1
sed -i "s/^ \{0,\}/    - /" $Proxy_Group 2>/dev/null 


yml_servers_add()
{
	
	local section="$1"
	config_get "name" "$section" "name" ""
	config_list_foreach "$section" "groups" set_groups "$name" "$2"
	
}

set_groups()
{

	if [ "$1" = "$3" ]; then
	   echo "    - \"${2}\"" >>$GROUP_FILE 2>/dev/null 
	fi

}

set_other_groups()
{

   if [ "${1}" = "DIRECT" ]||[ "${1}" = "REJECT" ];then
   echo "    - ${1}" >>$GROUP_FILE 2>/dev/null 
   elif [ "${1}" = "ALL" ];then
   cat $Proxy_Group >> $GROUP_FILE 2>/dev/nul
   else
   echo "    - \"${1}\"" >>$GROUP_FILE 2>/dev/null 
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

   if [ -z "$type" ]; then
      return
   fi
   
   if [ -z "$name" ]; then
      return
   fi
   
   echo "- name: $name" >>$GROUP_FILE 2>/dev/null 
   echo "  type: $type" >>$GROUP_FILE 2>/dev/null 

  if [ "$type" == "url-test" ] || [ "$type" == "load-balance" ] || [ "$type" == "fallback" ]; then
      echo "  proxies:" >>$GROUP_FILE 2>/dev/null 
      #cat $Proxy_Group >> $GROUP_FILE 2>/dev/null
   else
      echo "  proxies:" >>$GROUP_FILE 2>/dev/null 
   fi       
 
   if [ "$name" != "$old_name" ]; then
      sed -i "s/,${old_name}$/,${name}#d/g" $CONFIG_FILE 2>/dev/null
      sed -i "s/:${old_name}$/:${name}#d/g" $CONFIG_FILE 2>/dev/null
      sed -i "s/\'${old_name}\'/\'${name}\'/g" $CFG_FILE 2>/dev/null
      config_load "clash"
   fi
   
   config_list_foreach "$section" "other_group" set_other_groups 
   config_foreach yml_servers_add "servers" "$name" 
   
   [ ! -z "$test_url" ] && {
   	echo "  url: $test_url" >>$GROUP_FILE 2>/dev/null 
   }
   [ ! -z "$test_interval" ] && {
   echo "  interval: \"$test_interval\"" >>$GROUP_FILE 2>/dev/null 
   }
}


config_load clash
config_foreach yml_groups_set "groups"


if [ "$(ls -l $GROUP_FILE|awk '{print $5}')" -ne 0 ]; then
sed -i "1i\  " $GROUP_FILE 2>/dev/null 
sed -i "2i\Proxy Group:" $GROUP_FILE 2>/dev/null 
fi



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

		
cat >> "$TEMP_FILE" <<-EOF
#config-start-here
EOF

		sed -i "1i\port: ${http_port}" $TEMP_FILE 2>/dev/null
		sed -i "/port: ${http_port}/a\socks-port: ${socks_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/socks-port: ${socks_port}/a\redir-port: ${redir_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/redir-port: ${redir_port}/a\allow-lan: ${allow_lan}" $TEMP_FILE 2>/dev/null 
		if [ $allow_lan == "true" ];  then
		sed -i "/allow-lan: ${allow_lan}/a\bind-address: \"${bind_addr}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/bind-address: \"${bind_addr}\"/a\mode: Rule" $TEMP_FILE 2>/dev/null
		sed -i "/mode: Rule/a\log-level: ${log_level}" $TEMP_FILE 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $TEMP_FILE 2>/dev/null 
		sed -i "external-ui: \"/usr/share/clash/dashboard\"/a\  " $TEMP_FILE 2>/dev/null 
		sed -i "   /a\   " $TEMP_FILE 2>/dev/null
		else
		sed -i "/allow-lan: ${allow_lan}/a\mode: Rule" $TEMP_FILE 2>/dev/null
		sed -i "/mode: Rule/a\log-level: ${log_level}" $TEMP_FILE 2>/dev/null 
		sed -i "/log-level: ${log_level}/a\external-controller: 0.0.0.0:${dash_port}" $TEMP_FILE 2>/dev/null 
		sed -i "/external-controller: 0.0.0.0:${dash_port}/a\secret: \"${da_password}\"" $TEMP_FILE 2>/dev/null 
		sed -i "/secret: \"${da_password}\"/a\external-ui: \"/usr/share/clash/dashboard\"" $TEMP_FILE 2>/dev/null 
		fi
		sed -i '/#config-start-here/ d' $TEMP_FILE 2>/dev/null

		
cat $DNS_FILE >> $TEMP_FILE  2>/dev/null

cat $SERVER_FILE >> $TEMP_FILE  2>/dev/null

cat $GROUP_FILE >> $TEMP_FILE 2>/dev/null

cat $TEMP_FILE $CONFIG_YAML_RULE > $CONFIG_YAML 2>/dev/null

sed -i "/Rule:/i\     " $CONFIG_YAML 2>/dev/null
rm -rf $TEMP_FILE $GROUP_FILE $Proxy_Group $CONFIG_FILE


 	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Completed Creating Custom Config.. " >$REAL_LOG 
		 sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
    	 echo "创建自定义配置完成..." >$REAL_LOG
		  sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	fi
	
	if [ $lang == "en" ] || [ $lang == "auto" ];then
		echo "Updating Configuration Completed" >$REAL_LOG
		  sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	elif [ $lang == "zh_cn" ];then
		echo "更新配置完成" >$REAL_LOG
		  sleep 2
			echo "Clash for OpenWRT" >$REAL_LOG
	fi

mv /usr/share/clash/v2ssr/config.bak /etc/config/clash 2>/dev/null
sleep 1

	if pidof clash >/dev/null; then
		/etc/init.d/clash restart 2>/dev/null
	fi
	
fi
rm -rf $SERVER_FILE

fi
