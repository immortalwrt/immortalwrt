#!/bin/bash

. $IPKG_INSTROOT/lib/functions.sh
. /usr/share/libubox/jshn.sh

CONFIG=passwall
LOCK_FILE=/var/lock/${CONFIG}_subscription.lock
Date=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE=/var/log/$CONFIG.log

config_t_get() {
	local index=0
	[ -n "$3" ] && index=$3
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	#echo ${ret:=$3}
	echo $ret
}

urldecode() { : "${*//+/ }"; echo -e "${_//%/\\x}"; }

decode_url_link() {
	link=$1
	num=$2
	len=$((${#link}-$num))
	mod4=$(($len%4))
	if [ "$mod4" -gt 0 ]; then
		var="===="
		newlink=${link}${var:$mod4}
		echo -n "$newlink" | sed 's/-/+/g; s/_/\//g' | base64 -d -i 2> /dev/null
	else
		echo -n "$link" | sed 's/-/+/g; s/_/\//g' | base64 -d -i 2> /dev/null
	fi
}

start_subscribe() {
	local enabled
	local url
	config_get enabled $1 enabled
	config_get url $1 url
	[ "$enabled" == "1" ] && {
		[ -n "$url" -a "$url" != "" ] && {
			local addnum_ss=0
			local updatenum_ss=0
			local delnum_ss=0
			local addnum_ssr=0
			local updatenum_ssr=0
			local delnum_ssr=0
			local addnum_v2ray=0
			local updatenum_v2ray=0
			local delnum_v2ray=0
			local addnum_trojan=0
			local updatenum_trojan=0
			local delnum_trojan=0
			config_get subscrib_remark $1 remark
			let index+=1
			echo "$Date: 正在订阅：$url" >> $LOG_FILE
			result=$(/usr/bin/curl --connect-timeout 5 --retry 1 -sL $url)
			[ "$?" != 0 ] || [ -z "$result" ] && {
				result=$(/usr/bin/wget --no-check-certificate --timeout=5 -t 1 -O- $url)
				[ "$?" != 0 ] || [ -z "$result" ] && echo "$Date: 订阅失败：$url，请检测订阅链接是否正常或使用代理尝试！" >> $LOG_FILE && continue
			}
			file="/var/${CONFIG}_sub/$index"
			echo "$result" > $file
			
			get_local_nodes
			
			if [ $(expr "$result" : "ssd://") == 0 ];then
				[ -z "$(du -sh $file 2> /dev/null)" ] && echo "$Date: 订阅失败：$url，解密失败！" >> $LOG_FILE && continue
				decode_link=$(cat "$file" | base64 -d 2> /dev/null)
				maxnum=$(echo -n "$decode_link" | grep "MAX=" | awk -F"=" '{print $2}')
				if [ -n "$maxnum" ]; then
					decode_link=$(echo -n "$decode_link" | sed '/MAX=/d' | shuf -n${maxnum})
				else
					decode_link=$(echo -n "$decode_link")
				fi
				
				[ -z "$decode_link" ] && continue
				for link in $decode_link
				do
					if expr "$link" : "ss://";then
						link_type="ss"
						new_link=$(echo -n "$link" | sed 's/ss:\/\///g')
					elif expr "$link" : "ssr://";then
						link_type="ssr"
						new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
					elif expr "$link" : "vmess://";then
						link_type="v2ray"
						new_link=$(echo -n "$link" | sed 's/vmess:\/\///g')
					elif expr "$link" : "trojan://";then
						link_type="trojan"
						new_link=$(echo -n "$link" | sed 's/trojan:\/\///g')
					fi
					[ -z "$link_type" ] && continue
					get_remote_config "$link_type" "$new_link"
				done
			else
				link=$result
				link_type="ssd"
				new_link=$(echo -n "$link" | sed 's/ssd:\/\///g')
				[ -z "$link_type" ] && continue
				get_remote_config "$link_type" "$new_link"
			fi
			[ "$addnum_ss" -gt 0 ] || [ "$updatenum_ss" -gt 0 ] || [ "$delnum_ss" -gt 0 ] && echo "$Date: $subscrib_remark： SS节点新增 $addnum_ss 个，修改 $updatenum_ss 个，删除 $delnum_ss 个。" >> $LOG_FILE
			[ "$addnum_ssr" -gt 0 ] || [ "$updatenum_ssr" -gt 0 ] || [ "$delnum_ssr" -gt 0 ] && echo "$Date: $subscrib_remark： SSR节点新增 $addnum_ssr 个，修改 $updatenum_ssr 个，删除 $delnum_ssr 个。" >> $LOG_FILE
			[ "$addnum_v2ray" -gt 0 ] || [ "$updatenum_v2ray" -gt 0 ] || [ "$delnum_v2ray" -gt 0 ] && echo "$Date: $subscrib_remark： V2ray节点新增 $addnum_v2ray 个，修改 $updatenum_v2ray 个，删除 $delnum_v2ray 个。" >> $LOG_FILE
			[ "$addnum_trojan" -gt 0 ] || [ "$updatenum_trojan" -gt 0 ] || [ "$delnum_trojan" -gt 0 ] && echo "$Date: $subscrib_remark： Trojan节点新增 $addnum_trojan 个，修改 $updatenum_trojan 个，删除 $delnum_trojan 个。" >> $LOG_FILE
		}
	}
}

ss_decode() {
	temp_link=$1
	if [[ "$(echo $temp_link | grep "@")" != "" ]]; then
		link1=$(decode_url_link $(echo -n "$temp_link" | awk -F '@' '{print $1}') 1)
		link2=$(echo -n "$temp_link" | awk -F '@' '{print $2}')
		echo -n "${link1}@${link2}"
	elif [[ "$(echo $temp_link | grep "#")" != "" ]]; then
		link1=$(decode_url_link $(echo -n "$temp_link" | awk -F '#' '{print $1}') 1)
		link2=$(echo -n "$temp_link" | awk -F '#' '{print $2}')
		echo -n "${link1}#${link2}"
	else
		link1=$(decode_url_link $(echo -n "$temp_link" ) 1)
		echo -n "$link1"
	fi
}

get_node_index(){
	[ -f "/etc/config/$CONFIG" ] && {
        nodes_index=$(uci show $CONFIG | grep -c "=nodes")
	}	
}

get_local_nodes(){
	[ -f "/etc/config/$CONFIG" ] && [ "`uci show $CONFIG | grep -c 'is_sub'`" -gt 0 ] && {
		get_node_index
		for i in `seq $nodes_index -1 1`
		do
			[ "$(uci show $CONFIG.@nodes[$(($i-1))]|grep -c "is_sub")" -eq 1 ] && {
				if [ ! -f "/usr/share/${CONFIG}/sub/all_localnodes" ]; then
					echo $(config_t_get nodes address $(($i-1))) > /usr/share/${CONFIG}/sub/all_localnodes
				else
					echo $(config_t_get nodes address $(($i-1))) >> /usr/share/${CONFIG}/sub/all_localnodes
				fi
			}
		done
	}
}

get_remote_config(){
	add_mode="$subscrib_remark"
	[ -n "$3" ] && add_mode="导入"
	new_node_type=$(echo $1 | tr '[a-z]' '[A-Z]')
	if [ "$1" == "ss" ]; then
		decode_link=$(ss_decode $2)
		ss_encrypt_method=$(echo "$decode_link" | awk -F ':' '{print $1}')
		password=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $1}')
		node_address=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $2}')
		
		plugin_tmp=$(echo "$decode_link" | awk -F '\\/\\?' '{print $2}' | awk -F '#' '{print $1}')
		if [ "$plugin_tmp" != "" ]; then 
			plugin_tmp=$(urldecode $plugin_tmp)
			plugin=$(echo "$plugin_tmp" | awk -F 'plugin=' '{print $2}' | awk -F ';' '{print $1}')
			plugin_options=$(echo "$plugin_tmp" | awk -F "$plugin;" '{print $2}' | awk -F '&' '{print $1}')
			node_port=$(echo "$decode_link" | awk -F '@' '{print $2}' | awk -F '\\/\\?' '{print $1}' | awk -F ':' '{print $2}')
		else
			node_port=$(echo "$decode_link" | awk -F '@' '{print $2}' | awk -F '#' '{print $1}' | awk -F ':' '{print $2}')
		fi

		[ -n "$plugin" -a "$plugin" == "simple-obfs" ] && plugin=obfs-local
		remarks=$(urldecode $(echo "$decode_link" | awk -F '#' '{print $2}'))
	elif [ "$1" == "ssr" ]; then
		decode_link=$(decode_url_link $2 1)
		node_address=$(echo "$decode_link" | awk -F ':' '{print $1}')
		node_port=$(echo "$decode_link" | awk -F ':' '{print $2}')
		protocol=$(echo "$decode_link" | awk -F ':' '{print $3}')
		ssr_encrypt_method=$(echo "$decode_link" | awk -F ':' '{print $4}')
		obfs=$(echo "$decode_link" | awk -F ':' '{print $5}')
		password=$(decode_url_link $(echo "$decode_link" | awk -F ':' '{print $6}' | awk -F '/' '{print $1}') 0)
		
		obfsparm_temp=$(echo "$decode_link" |grep -Eo "obfsparam.+" |sed 's/obfsparam=//g'|awk -F'&' '{print $1}')
		[ -n "$obfsparm_temp" ] && obfsparam=$(decode_url_link $obfsparm_temp 0) || obfsparam=''
		protoparam_temp=$(echo "$decode_link" |grep -Eo "protoparam.+" |sed 's/protoparam=//g'|awk -F'&' '{print $1}')
		[ -n "$protoparam_temp" ] && protoparam=$(decode_url_link $protoparam_temp 0) || protoparam=''
		remarks_temp=$(echo "$decode_link" |grep -Eo "remarks.+" |sed 's/remarks=//g'|awk -F'&' '{print $1}')
		[ -n "$remarks_temp" ] && remarks="$(decode_url_link $remarks_temp 0)"
		group_temp=$(echo "$decode_link" |grep -Eo "group.+" |sed 's/group=//g'|awk -F'&' '{print $1}')
		[ -n "$group_temp" ] && group="$(decode_url_link $group_temp 0)"
	elif [ "$1" == "v2ray" ]; then
		decode_link=$(decode_url_link $2 1)
		json_load "$decode_link"
		json_get_var json_v v
		json_get_var json_ps ps
		json_get_var json_node_address add
		json_get_var json_node_port port
		json_get_var json_id id
		json_get_var json_aid aid
		json_get_var json_security security
		json_get_var json_net net
		json_get_var json_type type
		json_get_var json_transport net
		json_get_var json_tls tls
		json_get_var json_host host
		json_get_var json_path path
		
		if [ "$json_tls" == "tls" -o "$json_tls" == "1" ]; then
			json_tls="tls"
		else
			json_tls="none"
		fi
		
		remarks="${json_ps}"
		node_address=$json_node_address
		node_port=$json_node_port
	elif [ "$1" == "trojan" ]; then
		link="$2"
		node_password=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $1}')
		node_address=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $2}' | awk -F ':' '{print $1}')
		node_port=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $2}' | awk -F ':' '{print $2}')
		remarks="${node_address}:${node_port}"
	elif [ "$1" == "ssd" ]; then
		link_type="ss"
		new_node_type=$(echo $link_type | tr '[a-z]' '[A-Z]')
		decode_link=$(decode_url_link $2 1)
		json_load "$decode_link"
		json_get_var json_airport airport
		json_get_var json_port port
		json_get_var json_encryption encryption
		json_get_var json_password password
		json_get_var json_traffic_used traffic_used
		json_get_var json_traffic_total traffic_total
		json_get_var json_expiry expiry
		json_get_var json_url url
		json_get_var json_plugin plugin
		json_get_var json_plugin_options plugin_options
		
		ss_encrypt_method=$json_encryption
		password=$json_password
		plugin=$json_plugin
		plugin_options=$json_plugin_options
		
		[ -n "$plugin" -a "$plugin" == "simple-obfs" ] && plugin=obfs-local

		if json_get_type Type servers && [ "$Type" == array ]
		then
			json_select servers
			idx=1
			while json_get_type Type "$idx" && [ "$Type" == object ]
			do
				json_select $idx
				json_get_var json_server server
				json_get_var json_server_id id
				json_get_var json_server_ratio ratio
				json_get_var json_server_remarks remarks
				
				remarks="${json_server_remarks}"
				node_address=$json_server
				node_port=$json_port
				
				idx=$(expr $idx + 1)
				json_select ..
				
				node_address=$(echo -n $node_address | awk '{print gensub(/[^!-~]/,"","g",$0)}')
				node_address=$(echo -n $node_address | grep -F ".")
				[ -z "$node_address" -o "$node_address" == "" ] && return
				
				[ -z "$remarks" -o "$remarks" == "" ] && remarks="${node_address}:${node_port}"
				tmp=$(echo $remarks | grep -E "过期时间|剩余流量|QQ群|官网")
				[ -n "$tmp" ] && {
					echo "$Date: 丢弃无用节点：$tmp" >> $LOG_FILE
					return
				}
				
				# 把全部节点节点写入文件 /usr/share/${CONFIG}/sub/all_onlinenodes
				if [ ! -f "/usr/share/${CONFIG}/sub/all_onlinenodes" ]; then
					echo $node_address > /usr/share/${CONFIG}/sub/all_onlinenodes
				else
					echo $node_address >> /usr/share/${CONFIG}/sub/all_onlinenodes
				fi
				
				update_config
			done
			return
		fi
	fi
	
	node_address=$(echo -n $node_address | awk '{print gensub(/[^!-~]/,"","g",$0)}')
	node_address=$(echo -n $node_address | grep -F ".")
	[ -z "$node_address" -o "$node_address" == "" ] && return
	
	[ -z "$remarks" -o "$remarks" == "" ] && remarks="${node_address}:${node_port}"
	tmp=$(echo $remarks | grep -E "过期时间|剩余流量|QQ群|官网")
	[ -n "$tmp" ] && {
		echo "$Date: 丢弃无用节点：$tmp" >> $LOG_FILE
		return
	}
	
	# 把全部节点节点写入文件 /usr/share/${CONFIG}/sub/all_onlinenodes
	if [ ! -f "/usr/share/${CONFIG}/sub/all_onlinenodes" ]; then
		echo $node_address > /usr/share/${CONFIG}/sub/all_onlinenodes
	else
		echo $node_address >> /usr/share/${CONFIG}/sub/all_onlinenodes
	fi
	
	update_config
}

add_nodes(){
	if [ "$1" == "add" ]; then
		get_node_index
		uci add $CONFIG nodes
	elif [ "$1" == "update" ]; then
		nodes_index=$update_index
	fi
	uci_set="uci set $CONFIG.@nodes[$nodes_index]."
	[ "$add_mode" != "导入" ] && ${uci_set}is_sub="is_sub"
	if [ "$2" == "ss" ]; then
		${uci_set}add_mode="$add_mode"
		${uci_set}remarks="$remarks"
		${uci_set}type="SS"
		${uci_set}address="$node_address"
		${uci_set}use_ipv6=0
		${uci_set}port="$node_port"
		${uci_set}password="$password"
		${uci_set}ss_encrypt_method="$ss_encrypt_method"
		${uci_set}timeout=300
		${uci_set}tcp_fast_open=false
		[ -n "$plugin" ] && ${uci_set}ss_plugin="$plugin"
		[ -n "$plugin_options" ] && ${uci_set}ss_plugin_opts="$plugin_options"
		
		if [ "$1" == "add" ]; then
			let addnum_ss+=1
		elif [ "$1" == "update" ]; then
			let updatenum_ss+=1
		fi
		
	elif [ "$2" == "ssr" ]; then
		${uci_set}add_mode="$add_mode"
		${uci_set}remarks="$remarks"
		${uci_set}type="SSR"
		${uci_set}address="$node_address"
		${uci_set}use_ipv6=0
		${uci_set}port="$node_port"
		${uci_set}password="$password"
		${uci_set}ssr_encrypt_method="$ssr_encrypt_method"
		${uci_set}protocol="$protocol"
		${uci_set}protocol_param="$protoparam"
		${uci_set}obfs="$obfs"
		${uci_set}obfs_param="$obfsparam"
		${uci_set}timeout=300
		${uci_set}tcp_fast_open=false
		${uci_set}group="$group"
		
		if [ "$1" == "add" ]; then
			let addnum_ssr+=1
		elif [ "$1" == "update" ]; then
			let updatenum_ssr+=1
		fi
		
	elif [ "$2" == "v2ray" ]; then
		${uci_set}add_mode="$add_mode"
		${uci_set}remarks="$remarks"
		${uci_set}type="V2ray"
		${uci_set}v2ray_protocol="vmess"
		${uci_set}address="$node_address"
		${uci_set}use_ipv6=0
		${uci_set}port="$json_node_port"
		${uci_set}v2ray_security="auto"
		${uci_set}v2ray_VMess_id="$json_id"
		${uci_set}v2ray_VMess_alterId="$json_aid"
		${uci_set}v2ray_VMess_level="$json_v"
		${uci_set}v2ray_transport="$json_net"
		${uci_set}v2ray_stream_security="$json_tls"
		${uci_set}v2ray_tcp_guise="$json_type"
		${uci_set}v2ray_ws_host="$json_host"
		${uci_set}v2ray_ws_path="$json_path"
		${uci_set}v2ray_h2_host="$json_host"
		${uci_set}v2ray_h2_path="$json_path"
		${uci_set}tls_allowInsecure=1
		
		if [ "$1" == "add" ]; then
			let addnum_v2ray+=1
		elif [ "$1" == "update" ]; then
			let updatenum_v2ray+=1
		fi
		
	elif [ "$2" == "trojan" ]; then
		${uci_set}add_mode="$add_mode"
		${uci_set}remarks="$remarks"
		${uci_set}type="Trojan"
		${uci_set}address="$node_address"
		${uci_set}port="$node_port"
		${uci_set}password="$node_password"
		
		if [ "$1" == "add" ]; then
			let addnum_trojan+=1
		elif [ "$1" == "update" ]; then
			let updatenum_trojan+=1
		fi
		
	fi
	uci commit $CONFIG
}

update_config(){
	[ -z "$remarks" -o "$remarks" == "" ] && return
	indexs=$(uci show $CONFIG | grep "@nodes" | grep "remarks=" | grep -F "$remarks" | cut -d '[' -f2|cut -d ']' -f1)
	if [ -z "$indexs" ]; then
		add_nodes add "$link_type"
	else
		action="add"
		for index in $indexs
		do
			local is_sub=$(uci -q get $CONFIG.@nodes[$index].is_sub)
			[ -z "$is_sub" -o "$is_sub" == "" ] && return
			local old_node_type=$(uci -q get $CONFIG.@nodes[$index].type | tr '[a-z]' '[A-Z]')
			if [ -n "$old_node_type" -a "$old_node_type" == "$new_node_type" ]; then
				action="update"
				update_index=$index
				break
			fi
		done
		if [ "$action" == "add" ]; then
			add_nodes add "$link_type"
		elif [ "$action" == "update" ]; then
			add_nodes update "$link_type"
		fi
	fi
}

del_config(){
	# 删除订阅节点已经不存在的节点
	for localaddress in $(cat /usr/share/${CONFIG}/sub/all_localnodes)
	do
		[ "`cat /usr/share/${CONFIG}/sub/all_onlinenodes |grep -c "$localaddress"`" -eq 0 ] && {
			for localindex in $(uci show $CONFIG | grep -w "$localaddress" | grep -w "address=" |cut -d '[' -f2|cut -d ']' -f1)
			do
				del_type=$(uci -q get $CONFIG.@nodes[$localindex].type)
				uci delete $CONFIG.@nodes[$localindex]
				uci commit $CONFIG
				if [ "$del_type" == "SS" ]; then
					let delnum_ss+=1 #删除该节点
				elif [ "$del_type" == "SSR" ]; then
					let delnum_ssr+=1 #删除该节点
				elif [ "$del_type" == "V2ray" ]; then
					let delnum_v2ray+=1 #删除该节点
				elif [ "$del_type" == "Trojan" ]; then
					let delnum_trojan=1 #删除该节点
				fi
				
			done
		}
	done
}

del_all_config(){
	get_node_index
	[ "`uci show $CONFIG | grep -c 'is_sub'`" -eq 0 ] && exit 0
	TCP_NODE_NUM=$(uci -q get $CONFIG.@global_other[0].tcp_node_num)
	[ -z "$TCP_NODE_NUM" ] && TCP_NODE_NUM=1
	for i in $(seq 1 $TCP_NODE_NUM); do
		eval TCP_NODE$i=$(config_t_get global tcp_node$i)
	done

	UDP_NODE_NUM=$(uci -q get $CONFIG.@global_other[0].udp_node_num)
	[ -z "$UDP_NODE_NUM" ] && UDP_NODE_NUM=1
	for i in $(seq 1 $UDP_NODE_NUM); do
		eval UDP_NODE$i=$(config_t_get global udp_node$i)
	done

	SOCKS5_NODE_NUM=$(uci -q get $CONFIG.@global_other[0].socks5_node_num)
	[ -z "$SOCKS5_NODE_NUM" ] && SOCKS5_NODE_NUM=1
	for i in $(seq 1 $SOCKS5_NODE_NUM); do
		eval SOCKS5_NODE$i=$(config_t_get global socks5_node$i)
	done
	
	[ "$UDP_NODE1" == "default" ] && UDP_NODE1=$TCP_NODE1
	
	is_stop=0
	
	for i in $(seq 1 $TCP_NODE_NUM); do
		eval node=\$TCP_NODE$i
		[ -n "$node" -a "$node" != "nil" ] && {
			is_sub_node=`uci -q get $CONFIG.$node.is_sub`
			[ -n "$is_sub_node" ] && {
				is_stop=1
				uci set $CONFIG.@global[0].tcp_node$i="nil" && uci commit $CONFIG
			}
		}
	done
	
	for i in $(seq 1 $UDP_NODE_NUM); do
		eval node=\$UDP_NODE$i
		[ "$node" != "nil" ] && {
			is_sub_node=`uci -q get $CONFIG.$node.is_sub`
			[ -n "$is_sub_node" ] && {
				is_stop=1
				uci set $CONFIG.@global[0].udp_node$i="nil" && uci commit $CONFIG
			}
		}
	done
	
	for i in $(seq 1 $SOCKS5_NODE_NUM); do
		eval node=\$SOCKS5_NODE$i
		[ "$node" != "nil" ] && {
			is_sub_node=`uci -q get $CONFIG.$node.is_sub`
			[ -n "$is_sub_node" ] && {
				is_stop=1
				uci set $CONFIG.@global[0].socks5_node$i="nil" && uci commit $CONFIG
			}
		}
	done
	
	for i in `seq $nodes_index -1 1`
	do
		[ "$(uci show $CONFIG.@nodes[$(($i-1))] | grep -c 'is_sub')" -eq 1 ] && uci delete $CONFIG.@nodes[$(($i-1))] && uci commit $CONFIG
	done
	
	[ "$is_stop" == 1 ] && /etc/init.d/$CONFIG restart
}

add() {
	base64=$(command -v base64)
	[ -z "$base64" ] && echo "$Date: 找不到Base64程序，请安装后重试！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	LINKS=$(cat /tmp/links.conf 2>/dev/null)
	[ -n "$LINKS" ] && {
		[ -f "$LOCK_FILE" ] && return 3
		touch "$LOCK_FILE"
		mkdir -p /usr/share/${CONFIG}/sub && rm -f /usr/share/${CONFIG}/sub/*
		for link in $LINKS
		do
			if expr "$link" : "ss://";then
				link_type="ss"
				new_link=$(echo -n "$link" | sed 's/ss:\/\///g')
			elif expr "$link" : "ssr://";then
				link_type="ssr"
				new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "vmess://";then
				link_type="v2ray"
				new_link=$(echo -n "$link" | sed 's/vmess:\/\///g')
			elif expr "$link" : "trojan://";then
				link_type="trojan"
				new_link=$(echo -n "$link" | sed 's/trojan:\/\///g')
			fi
			[ -z "$link_type" ] && continue
			get_remote_config "$link_type" "$new_link" 1
		done
		[ -f "/usr/share/${CONFIG}/sub/all_onlinenodes" ] && rm -f /usr/share/${CONFIG}/sub/all_onlinenodes
	}
	rm -f /tmp/links.conf
	rm -f "$LOCK_FILE"
	exit 0
}

start() {
	# 防止并发开启服务
	[ -f "$LOCK_FILE" ] && return 3
	touch "$LOCK_FILE"
	base64=$(command -v base64)
	[ -z "$base64" ] && echo "$Date: 找不到Base64程序，请安装后重试！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	has_subscribe=$(uci show $CONFIG | grep @subscribe_list | grep enabled=\'1\')
	[ -z "$has_subscribe" -o "$has_subscribe" = "" ] && echo "$Date: 没有订阅地址或没启用！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	
	echo "$Date: 开始订阅..." >> $LOG_FILE
	mkdir -p /var/${CONFIG}_sub && rm -f /var/${CONFIG}_sub/*
	mkdir -p /usr/share/${CONFIG}/sub && rm -f /usr/share/${CONFIG}/sub/*
	index=0
	config_load $CONFIG
	config_foreach start_subscribe "subscribe_list"
	
	[ -f "/usr/share/${CONFIG}/sub/all_localnodes" ] && del_config
	echo "$Date: 订阅完毕..." >> $LOG_FILE
	rm -f "$LOCK_FILE"
	exit 0
}

stop() {
	[ "`uci show $CONFIG | grep -c 'is_sub'`" -gt 0 ] && {
		del_all_config
		echo "$Date: 在线订阅节点已全部删除" >> $LOG_FILE
	}
	rm -rf /var/${CONFIG}_sub
	rm -rf /usr/share/${CONFIG}/sub
	rm -f "$LOCK_FILE"
	exit 0
}

case $1 in
stop)
	stop
	;;
add)
	add
	;;
*)
	start
	;;
esac
