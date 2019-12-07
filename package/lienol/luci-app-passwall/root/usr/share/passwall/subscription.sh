#!/bin/sh

. /usr/share/libubox/jshn.sh

CONFIG=passwall
LOCK_FILE=/var/lock/${CONFIG}_subscription.lock
Date=$(date "+%Y-%m-%d %H:%M:%S")
LOG_FILE=/var/log/$CONFIG.log

config_t_get() {
	local index=0
	[ -n "$3" ] && index=$3
	local ret=$(uci get $CONFIG.@$1[$index].$2 2>/dev/null)
	#echo ${ret:=$3}
	echo $ret
}

decode_url_link() {
	link=$1
	num=$2
	len=$((${#link}-$num))
	mod4=$(($len%4))
	if [ "$mod4" -gt 0 ]; then
		var="===="
		newlink=${link}${var:$mod4}
		echo -n "$newlink" | sed 's/-/+/g; s/_/\//g' | /usr/bin/base64 -d -i 2> /dev/null
	else
		echo -n "$link" | sed 's/-/+/g; s/_/\//g' | /usr/bin/base64 -d -i 2> /dev/null
	fi
}

get_node_index(){
	[ -f "/etc/config/$CONFIG" ] && {
        nodes_index=$(uci show $CONFIG | grep -c "=nodes")
	}	
}

get_local_nodes(){
	[ -f "/etc/config/$CONFIG" ] && [ "`uci show $CONFIG | grep -c 'sub_node'`" -gt 0 ] && {
		get_node_index
		for i in `seq $nodes_index -1 1`
		do
			[ "$(uci show $CONFIG.@nodes[$(($i-1))]|grep -c "sub_node")" -eq 1 ] && {
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
	isAdd=1
	add_mode="订阅"
	[ -n "$3" ] && add_mode="导入"
	group="sub_node"
	if [ "$1" == "ss" ]; then
		decode_link="$2"
		node_address=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $2}')
		node_port=$(echo "$decode_link" | awk -F ':' '{print $3}')
		ssr_encrypt_method=$(echo "$decode_link" | awk -F ':' '{print $1}')
		password=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $1}')
	elif [ "$1" == "ssr" ]; then
		decode_link="$2"
		node_address=$(echo "$decode_link" | awk -F ':' '{print $1}')
		node_address=$(echo $node_address |awk '{print gensub(/[^!-~]/,"","g",$0)}')
		[ -z "$node_address" -o "$node_address" == "" ] && isAdd=0
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
	elif [ "$1" == "v2ray" ]; then
		json_load "$2"
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
		
		if [ "$json_tls" == "1" ]; then
			json_tls="tls"
		else
			json_tls="none"
		fi
		
		remarks="${json_ps}"
		node_address=$json_node_address
	elif [ "$1" == "trojan" ]; then
		link="$2"
		node_password=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $1}')
		node_address=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $2}' | awk -F ':' '{print $1}')
		node_port=$(echo "$link" | sed 's/trojan:\/\///g' | awk -F '@' '{print $2}' | awk -F ':' '{print $2}')
		remarks="${node_address}:${node_port}"
	fi
	
	# 把全部服务器节点写入文件 /usr/share/${CONFIG}/sub/all_onlinenodes
	if [ ! -f "/usr/share/${CONFIG}/sub/all_onlinenodes" ]; then
		echo $node_address > /usr/share/${CONFIG}/sub/all_onlinenodes
	else
		echo $node_address >> /usr/share/${CONFIG}/sub/all_onlinenodes
	fi
	
}

add_nodes(){
	get_node_index
	uci_set="uci set $CONFIG.@nodes[$nodes_index]."
	uci add $CONFIG nodes > /dev/null
	[ -z "$3" ] && ${uci_set}group="$group"
	if [ "$2" == "ss" ]; then
		${uci_set}add_mode="$add_mode"
		${uci_set}remarks="$remarks"
		${uci_set}type="SSR"
		${uci_set}address="$node_address"
		${uci_set}use_ipv6=0
		${uci_set}port="$node_port"
		${uci_set}password="$password"
		${uci_set}ssr_encrypt_method="$ssr_encrypt_method"
		${uci_set}timeout=300
		${uci_set}tcp_fast_open=false
		
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
	[ "$isAdd" == 1 ] && {
		isadded_address=$(uci show $CONFIG | grep -c "remarks='$remarks'")
		if [ "$isadded_address" -eq 0 ]; then
			add_nodes add "$link_type"
		else
			index=$(uci show $CONFIG | grep -w "remarks='$remarks'" | cut -d '[' -f2|cut -d ']' -f1)
			local_port=$(config_t_get nodes port $index)
			local_vmess_id=$(config_t_get nodes v2ray_VMess_id $index)
			
			uci delete $CONFIG.@nodes[$index]
			add_nodes update "$link_type"
		fi
	}
}

del_config(){
	# 删除订阅服务器已经不存在的节点
	for localaddress in $(cat /usr/share/${CONFIG}/sub/all_localnodes)
	do
		[ "`cat /usr/share/${CONFIG}/sub/all_onlinenodes |grep -c "$localaddress"`" -eq 0 ] && {
			for localindex in $(uci show $CONFIG|grep -w "$localaddress" |grep -w "address=" |cut -d '[' -f2|cut -d ']' -f1)
			do
				del_type=$(uci get $CONFIG.@nodes[$localindex].type)
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
	[ "`uci show $CONFIG | grep -c 'sub_node'`" -eq 0 ] && exit 0
	current_tcp_node1=$(config_t_get global tcp_node1)
	is_sub_node=`uci -q get $CONFIG.$current_tcp_node1.group`
	for i in `seq $nodes_index -1 1`
	do
		[ "$(uci show $CONFIG.@nodes[$(($i-1))] | grep -c 'sub_node')" -eq 1 ] && uci delete $CONFIG.@nodes[$(($i-1))] && uci commit $CONFIG
	done
	[ -n "$is_sub_node" ] && {
		uci set $CONFIG.global[0].tcp_node1="nil"
		uci commit $CONFIG && /etc/init.d/$CONFIG stop
	}
}

add() {
	LINKS=$(cat /tmp/links.conf 2>/dev/null)
	[ -n "$LINKS" ] && {
		[ -f "$LOCK_FILE" ] && return 3
		touch "$LOCK_FILE"
		mkdir -p /usr/share/${CONFIG}/sub && rm -f /usr/share/${CONFIG}/sub/*
		for link in $LINKS
		do
			is_decode=1
			if expr "$link" : "ss://";then
				link_type="ss"
				new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "ssr://";then
				link_type="ssr"
				new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "vmess://";then
				link_type="v2ray"
				new_link=$(echo -n "$link" | sed 's/vmess:\/\///g')
			elif expr "$link" : "trojan://";then
				link_type="trojan"
				new_link=$(echo -n "$link" | sed 's/trojan:\/\///g')
				is_decode=0
			fi
			[ -z "$link_type" ] && continue
			[ "$is_decode" == 1 ] && new_link=$(decode_url_link $new_link 1)
			get_remote_config "$link_type" "$new_link" 1
			update_config
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
	addnum_ss=0
	updatenum_ss=0
	delnum_ss=0
	addnum_ssr=0
	updatenum_ssr=0
	delnum_ssr=0
	addnum_v2ray=0
	updatenum_v2ray=0
	delnum_v2ray=0
	addnum_trojan=0
	updatenum_trojan=0
	delnum_trojan=0
	subscribe_url=$(uci get $CONFIG.@global_subscribe[0].subscribe_url)  # 订阅地址
	[ -z "$subscribe_url" ] && echo "$Date: 订阅地址为空，订阅失败！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	
	echo "$Date: 开始订阅..." >> $LOG_FILE
	mkdir -p /var/${CONFIG}_sub && rm -f /var/${CONFIG}_sub/*
	#/usr/bin/wget --no-check-certificate --timeout=8 -t 2 $subscribe_url -P /var/${CONFIG}_sub
	status=$(curl -w %{http_code} --connect-timeout 10 $subscribe_url --silent -o /var/${CONFIG}_sub/sub)
	[ -z "$status" ] || [ "$status" == "404" ] || [ ! -d "/var/${CONFIG}_sub" ] || [ "$(ls /var/${CONFIG}_sub | wc -l)" -eq 0 ] && echo "$Date: 订阅链接下载失败，请重试！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	
	mkdir -p /usr/share/${CONFIG}/sub && rm -f /usr/share/${CONFIG}/sub/*
	get_local_nodes
	for file in /var/${CONFIG}_sub/*
	do
		[ -z "$(du -sh $file 2> /dev/null)" ] && echo "$Date: 订阅链接下载 $file 失败，请重试！" >> $LOG_FILE && continue
		decode_link=$(cat "$file" | /usr/bin/base64 -d 2> /dev/null)
		maxnum=$(echo -n "$decode_link" | grep "MAX=" | awk -F"=" '{print $2}')
		if [ -n "$maxnum" ]; then
			decode_link=$(echo -n "$decode_link" | sed '/MAX=/d' | shuf -n${maxnum})
		else
			decode_link=$(echo -n "$decode_link")
		fi
		
		[ -z "$decode_link" ] && continue
		for link in $decode_link
		do
			is_decode=1
			if expr "$link" : "ss://";then
				link_type="ss"
				new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "ssr://";then
				link_type="ssr"
				new_link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "vmess://";then
				link_type="v2ray"
				new_link=$(echo -n "$link" | sed 's/vmess:\/\///g')
			elif expr "$link" : "trojan://";then
				link_type="trojan"
				new_link=$(echo -n "$link" | sed 's/trojan:\/\///g')
				is_decode=0
			fi
			[ -z "$link_type" ] && continue
			[ "$is_decode" == 1 ] && new_link=$(decode_url_link $new_link 1)
			get_remote_config "$link_type" "$new_link"
			update_config
		done
	done
	[ -f "/usr/share/${CONFIG}/sub/all_localnodes" ] && del_config
	echo "$Date: 本次更新，SS新增服务器节点 $addnum_ss 个，修改 $updatenum_ss 个，删除 $delnum_ss 个。" >> $LOG_FILE
	echo "$Date: 本次更新，SSR新增服务器节点 $addnum_ssr 个，修改 $updatenum_ssr 个，删除 $delnum_ssr 个。" >> $LOG_FILE
	echo "$Date: 本次更新，V2ray新增服务器节点 $addnum_v2ray 个，修改 $updatenum_v2ray 个，删除 $delnum_v2ray 个。" >> $LOG_FILE
	echo "$Date: 本次更新，Trojan新增服务器节点 $addnum_trojan 个，修改 $updatenum_trojan 个，删除 $delnum_trojan 个。" >> $LOG_FILE
	echo "$Date: 订阅完毕..." >> $LOG_FILE
	rm -f "$LOCK_FILE"
	exit 0
}

stop() {
	[ "`uci show $CONFIG | grep -c 'sub_node'`" -gt 0 ] && {
		echo "$Date: 在线订阅节点已全部删除" >> $LOG_FILE
		del_all_config
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
