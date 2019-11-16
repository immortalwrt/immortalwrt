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

get_server_index(){
	[ -f "/etc/config/$CONFIG" ] && {
        servers_index=$(uci show $CONFIG | grep -c "=servers")
	}	
}

get_local_servers(){
	[ -f "/etc/config/$CONFIG" ] && [ "`uci show $CONFIG | grep -c 'sub_server'`" -gt 0 ] && {
		get_server_index
		for i in `seq $servers_index -1 1`
		do
			[ "$(uci show $CONFIG.@servers[$(($i-1))]|grep -c "sub_server")" -eq 1 ] && {
				if [ ! -f "/usr/share/${CONFIG}/sub/all_localservers" ]; then
					echo $(config_t_get servers server $(($i-1))) > /usr/share/${CONFIG}/sub/all_localservers
				else
					echo $(config_t_get servers server $(($i-1))) >> /usr/share/${CONFIG}/sub/all_localservers
				fi
			}
		done
	}
}

get_remote_config(){
	remarks="(订阅)"
	[ -n "$3" ] && remarks="(导入)"
	group="sub_server"
	if [ "$1" == "ss" ]; then
		decode_link="$2"
		server=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $2}')
		server_port=$(echo "$decode_link" | awk -F ':' '{print $3}')
		ssr_encrypt_method=$(echo "$decode_link" | awk -F ':' '{print $1}')
		password=$(echo "$decode_link" | awk -F ':' '{print $2}' | awk -F '@' '{print $1}')
		server_host=$server
	elif [ "$1" == "ssr" ]; then
		decode_link="$2"
		server=$(echo "$decode_link" | awk -F ':' '{print $1}')
		server_port=$(echo "$decode_link" | awk -F ':' '{print $2}')
		protocol=$(echo "$decode_link" | awk -F ':' '{print $3}')
		ssr_encrypt_method=$(echo "$decode_link" | awk -F ':' '{print $4}')
		obfs=$(echo "$decode_link" | awk -F ':' '{print $5}')
		password=$(decode_url_link $(echo "$decode_link" | awk -F ':' '{print $6}' | awk -F '/' '{print $1}') 0)
		
		obfsparm_temp=$(echo "$decode_link" |grep -Eo "obfsparam.+" |sed 's/obfsparam=//g'|awk -F'&' '{print $1}')
		[ -n "$obfsparm_temp" ] && obfsparam=$(decode_url_link $obfsparm_temp 0) || obfsparam=''
		protoparam_temp=$(echo "$decode_link" |grep -Eo "protoparam.+" |sed 's/protoparam=//g'|awk -F'&' '{print $1}')
		[ -n "$protoparam_temp" ] && protoparam=$(decode_url_link $protoparam_temp 0) || protoparam=''
		remarks_temp=$(echo "$decode_link" |grep -Eo "remarks.+" |sed 's/remarks=//g'|awk -F'&' '{print $1}')
		[ -n "$remarks_temp" ] && remarks="${remarks}$(decode_url_link $remarks_temp 0)"
		group_temp=$(echo "$decode_link" |grep -Eo "group.+" |sed 's/group=//g'|awk -F'&' '{print $1}')
		server_host=$server
	elif [ "$1" == "v2ray" ]; then
		json_load "$2"
		json_get_var json_v v
		json_get_var json_ps ps
		json_get_var json_server add
		json_get_var json_server_port port
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
		
		remarks="${remarks}${json_ps}"
		server_host=$json_server
	fi
	
	# 把全部服务器节点写入文件 /usr/share/${CONFIG}/sub/all_onlineservers
	if [ ! -f "/usr/share/${CONFIG}/sub/all_onlineservers" ]; then
		echo $server_host > /usr/share/${CONFIG}/sub/all_onlineservers
	else
		echo $server_host >> /usr/share/${CONFIG}/sub/all_onlineservers
	fi
	
}

add_servers(){
	get_server_index
	uci_set="uci set $CONFIG.@servers[$servers_index]."
	uci add $CONFIG servers > /dev/null
	[ -z "$2" ] && ${uci_set}group="$group"
	if [ "$1" == "ss" ]; then
		${uci_set}remarks="$remarks"
		${uci_set}server_type="SSR"
		${uci_set}server="$server_host"
		${uci_set}use_ipv6=0
		${uci_set}server_port="$server_port"
		${uci_set}password="$password"
		${uci_set}ssr_encrypt_method="$ssr_encrypt_method"
		${uci_set}timeout=300
		${uci_set}fast_open=false
		let addnum_ss+=1
	elif [ "$1" == "ssr" ]; then
		${uci_set}remarks="$remarks"
		${uci_set}server_type="SSR"
		${uci_set}server="$server_host"
		${uci_set}use_ipv6=0
		${uci_set}server_port="$server_port"
		${uci_set}password="$password"
		${uci_set}ssr_encrypt_method="$ssr_encrypt_method"
		${uci_set}protocol="$protocol"
		${uci_set}protocol_param="$protoparam"
		${uci_set}obfs="$obfs"
		${uci_set}obfs_param="$obfsparam"
		${uci_set}timeout=300
		${uci_set}fast_open=false
		let addnum_ssr+=1
	elif [ "$1" == "v2ray" ]; then
		${uci_set}remarks="$remarks"
		${uci_set}server_type="V2ray"
		${uci_set}v2ray_protocol="vmess"
		${uci_set}server="$json_server"
		${uci_set}use_ipv6=0
		${uci_set}server_port="$json_server_port"
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
		let addnum_v2ray+=1
	fi
	uci commit $CONFIG
}

update_config(){
	isadded_server=$(uci show $CONFIG | grep -c "server='$server_host'")
	if [ "$isadded_server" -eq 0 ]; then
		add_servers $link_type
	else
		index=$(uci show $CONFIG | grep -w "server='$server_host'" | cut -d '[' -f2|cut -d ']' -f1)
		local_server_port=$(config_t_get servers server_port $index)
		local_vmess_id=$(config_t_get servers v2ray_VMess_id $index)
		# To Do
		
	fi
}

del_config(){
	# 删除订阅服务器已经不存在的节点
	for localserver in $(cat /usr/share/${CONFIG}/sub/all_localservers)
	do
		[ "`cat /usr/share/${CONFIG}/sub/all_onlineservers |grep -c "$localserver"`" -eq 0 ] && {
			for localindex in $(uci show $CONFIG|grep -w "$localserver" |grep -w "server=" |cut -d '[' -f2|cut -d ']' -f1)
			do
				del_server_type=$(uci get $CONFIG.@servers[$localindex].server_type)
				uci delete $CONFIG.@servers[$localindex]
				uci commit $CONFIG
				if [ "$del_server_type" == "SS" ]; then
					let delnum_ss+=1 #删除该节点
				elif [ "$del_server_type" == "SSR" ]; then
					let delnum_ssr+=1 #删除该节点
				elif [ "$del_server_type" == "V2ray" ]; then
					let delnum_v2ray+=1 #删除该节点
				fi
				
			done
		}
	done
}

del_all_config(){
	get_server_index
	[ "`uci show $CONFIG | grep -c 'sub_server'`" -eq 0 ] && exit 0
	current_tcp_redir_server1=$(config_t_get global tcp_redir_server1)
	is_sub_server=`uci -q get $CONFIG.$current_tcp_redir_server1.group`
	for i in `seq $servers_index -1 1`
	do
		[ "$(uci show $CONFIG.@servers[$(($i-1))] | grep -c 'sub_server')" -eq 1 ] && uci delete $CONFIG.@servers[$(($i-1))] && uci commit $CONFIG
	done
	[ -n "$is_sub_server" ] && {
		uci set $CONFIG.global[0].tcp_redir_server1="nil"
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
			if [ -n "`echo -n "$link" | grep 'ss://'`" ]; then
				link_type="ss"
				new_link=`echo -n "$link" | sed 's/ss:\/\///g'`
			elif [ -n "`echo -n "$link" | grep 'ssr://'`" ]; then
				link_type="ssr"
				new_link=`echo -n "$link" | sed 's/ssr:\/\///g'`
			elif [ -n "`echo -n "$link" | grep 'vmess://'`" ]; then
				link_type="v2ray"
				new_link=`echo -n "$link" | sed 's/vmess:\/\///g'`
			fi
			[ -z "$link_type" ] && continue
			decode_link=$(decode_url_link $new_link 1)
			get_remote_config "$link_type" "$decode_link" 1
			is_added=$(uci show $CONFIG | grep -v "sub_server" | grep -c "server='$server_host'")
			[ "$is_added" -gt 0 ] && continue
			add_servers "$link_type" 1
		done
		[ -f "/usr/share/${CONFIG}/sub/all_onlineservers" ] && rm -f /usr/share/${CONFIG}/sub/all_onlineservers
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
	subscribe_url=$(uci get $CONFIG.@global_subscribe[0].subscribe_url)  # 订阅地址
	[ -z "$subscribe_url" ] && echo "$Date: 订阅地址为空，订阅失败！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	
	echo "$Date: 开始订阅..." >> $LOG_FILE
	mkdir -p /var/${CONFIG}_sub && rm -f /var/${CONFIG}_sub/*
	/usr/bin/wget --no-check-certificate --timeout=8 -t 2 $subscribe_url -P /var/${CONFIG}_sub
	[ ! -d "/var/${CONFIG}_sub" ] || [ "$(ls /var/${CONFIG}_sub | wc -l)" -eq 0 ] && echo "$Date: 订阅链接下载失败，请重试！" >> $LOG_FILE && rm -f "$LOCK_FILE" && exit 0
	
	mkdir -p /usr/share/${CONFIG}/sub && rm -f /usr/share/${CONFIG}/sub/*
	get_local_servers
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
			if expr "$link" : "ss://";then
				link_type="ss"
				link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "ssr://";then
				link_type="ssr"
				link=$(echo -n "$link" | sed 's/ssr:\/\///g')
			elif expr "$link" : "vmess://";then
				link_type="v2ray"
				link=$(echo -n "$link" | sed 's/vmess:\/\///g')
			fi
			[ -z "$link_type" ] && continue
			decode_link2=$(decode_url_link $link 1)
			get_remote_config "$link_type" "$decode_link2"
			update_config
		done
	done
	[ -f "/usr/share/${CONFIG}/sub/all_localservers" ] && del_config
	echo "$Date: 本次更新，SS新增服务器节点 $addnum_ss 个，修改 $updatenum_ss 个，删除 $delnum_ss 个。" >> $LOG_FILE
	echo "$Date: 本次更新，SSR新增服务器节点 $addnum_ssr 个，修改 $updatenum_ssr 个，删除 $delnum_ssr 个。" >> $LOG_FILE
	echo "$Date: 本次更新，V2ray新增服务器节点 $addnum_v2ray 个，修改 $updatenum_v2ray 个，删除 $delnum_v2ray 个。" >> $LOG_FILE
	echo "$Date: 订阅完毕..." >> $LOG_FILE
	rm -f "$LOCK_FILE"
	exit 0
}

stop() {
	[ "`uci show $CONFIG | grep -c 'sub_server'`" -gt 0 ] && {
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
