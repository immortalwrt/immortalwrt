#!/bin/sh

CONFIG=passwall
LOG_FILE=/var/log/$CONFIG.log

echolog() {
	local d="$(date "+%Y-%m-%d %H:%M:%S")"
	#echo -e "$d: $1"
	echo -e "$d: $1" >> $LOG_FILE
}

config_n_get() {
	local ret=$(uci -q get "${CONFIG}.${1}.${2}" 2>/dev/null)
	echo "${ret:=$3}"
}

config_t_get() {
	local index=0
	[ -n "$4" ] && index=$4
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	echo ${ret:=$3}
}

test_url() {
	local url=$1
	local try=1
	[ -n "$2" ] && try=$2
	local timeout=2
	[ -n "$3" ] && timeout=$3
	local extra_params=$4
	status=$(/usr/bin/curl -I -o /dev/null -skL $extra_params --connect-timeout $timeout --retry $try -w %{http_code} "$url")
	case "$status" in
		204|\
		200)
			status=200
		;;
	esac
	echo $status
}

test_proxy() {
	local try=3
	result=0
	status=$(test_url "https://www.google.com/generate_204" $try)
	if [ "$status" = "200" ]; then
		result=0
	else
		status2=$(test_url "https://www.baidu.com" $try)
		if [ "$status2" = "200" ]; then
			result=1
		else
			result=2
		fi
	fi
	echo $result
}

test_auto_switch() {
	local type=$1
	local index=$3
	local b_tcp_nodes=$4
	local now_node
	if [ -f "/var/etc/$CONFIG/id/${type}_${index}" ]; then
		now_node=$(cat /var/etc/$CONFIG/id/${type}_${index})
	else
		return 1
	fi

	status=$(test_proxy)
	if [ "$status" == 0 ]; then
		echolog "自动切换检测：${type}_${index}节点$(config_n_get $now_node type) $(config_n_get $now_node address) $(config_n_get $now_node port)正常。"
		#检测主节点是否能使用
		local main_node=$(config_t_get auto_switch tcp_main1)
		if [ "$now_node" != "$main_node" ]; then
			local node_type=$(echo $(config_n_get $main_node type) | tr 'A-Z' 'a-z')
			if [ "$node_type" == "socks" ]; then
				local node_address=$(config_n_get $main_node address)
				local node_port=$(config_n_get $main_node port)
				[ -n "$node_address" ] && [ -n "$node_port" ] && local curlx="socks5h://$node_address:$node_port"
			else
				local tmp_port=$(/usr/share/passwall/app.sh get_new_port 61080 tcp)
				/usr/share/passwall/app.sh run_socks "$main_node" "127.0.0.1" "$tmp_port" "/var/etc/passwall/auto_switch_$index.json" "10"
				local curlx="socks5h://127.0.0.1:$tmp_port"
			fi
			sleep 10s
			proxy_status=$(test_url "https://www.google.com/generate_204" 3 3 "-x $curlx")
			ps -w | grep -v "grep" | grep "/var/etc/passwall/auto_switch_$index.json" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1
			if [ "$proxy_status" -eq 200 ]; then
				#主节点正常，切换到主节点
				echolog "自动切换检测：${type}_${index}主节点正常，切换到主节点！"
				/usr/share/passwall/app.sh node_switch $type $2 $index $main_node
				return 0
			fi
		fi
		return 0
	elif [ "$status" == 2 ]; then
		echolog "自动切换检测：无法连接到网络，请检查网络是否正常！"
		return 2
	elif [ "$status" == 1 ]; then
		echolog "自动切换检测：${type}_${index}节点异常，开始切换节点！"
		
		#检测主节点是否能使用
		local main_node=$(config_t_get auto_switch tcp_main1)
		if [ "$now_node" != "$main_node" ]; then
			local node_type=$(echo $(config_n_get $main_node type) | tr 'A-Z' 'a-z')
			if [ "$node_type" == "socks" ]; then
				local node_address=$(config_n_get $main_node address)
				local node_port=$(config_n_get $main_node port)
				[ -n "$node_address" ] && [ -n "$node_port" ] && local curlx="socks5h://$node_address:$node_port"
			else
				local tmp_port=$(/usr/share/passwall/app.sh get_new_port 61080 tcp)
				/usr/share/passwall/app.sh run_socks "$main_node" "127.0.0.1" "$tmp_port" "/var/etc/passwall/auto_switch_$index.json" "10"
				local curlx="socks5h://127.0.0.1:$tmp_port"
			fi
			sleep 10s
			proxy_status=$(test_url "https://www.google.com/generate_204" 3 3 "-x $curlx")
			ps -w | grep -v "grep" | grep "/var/etc/passwall/auto_switch_$index.json" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1
			if [ "$proxy_status" -eq 200 ]; then
				#主节点正常，切换到主节点
				echolog "自动切换检测：${type}_${index}主节点正常，切换到主节点！"
				/usr/share/passwall/app.sh node_switch $type $2 $index $main_node
				return 0
			fi
		fi
		
		local new_node
		in_backup_nodes=$(echo $b_tcp_nodes | grep $now_node)
		# 判断当前节点是否存在于备用节点列表里
		if [ -z "$in_backup_nodes" ]; then
			# 如果不存在，设置第一个节点为新的节点
			new_node=$(echo $b_tcp_nodes | awk -F ' ' '{print $1}')
		else
			# 如果存在，设置下一个备用节点为新的节点
			#local count=$(expr $(echo $b_tcp_nodes | grep -o ' ' | wc -l) + 1)
			local next_node=$(echo $b_tcp_nodes | awk -F "$now_node" '{print $2}' | awk -F " " '{print $1}')
			if [ -z "$next_node" ]; then
				new_node=$(echo $b_tcp_nodes | awk -F ' ' '{print $1}')
			else
				new_node=$next_node
			fi
		fi
		/usr/share/passwall/app.sh node_switch $type $2 $index $new_node
		sleep 10s
		# 切换节点后等待10秒后再检测一次，如果还是不通继续切，直到可用为止
		status2=$(test_proxy)
		if [ "$status2" -eq 0 ]; then
			echolog "自动切换检测：${type}_${index}节点切换完毕！"
			return 0
		elif [ "$status2" -eq 1 ]; then
			test_auto_switch $1 $2 $3 "$4"
		elif [ "$status2" -eq 2 ]; then
			return 2
		fi
	fi
}

start() {
	(
		flock -xn 200
		[ "$?" != "0" ] && exit 0
		
		ENABLED=$(config_t_get global enabled 0)
		[ "$ENABLED" != 1 ] && return 1
		ENABLED=$(config_t_get auto_switch enable 0)
		[ "$ENABLED" != 1 ] && return 1
		delay=$(config_t_get auto_switch testing_time 1)
		sleep ${delay}m
		while [ "$ENABLED" -eq 1 ]
		do
			# TCP_NODE_NUM=$(config_t_get global_other tcp_node_num 1)
			# 暂时只能检测TCP1
			TCP_NODE_NUM=1
			for i in $(seq 1 $TCP_NODE_NUM); do
				eval TCP_NODE$i=\"$(config_t_get auto_switch tcp_node$i nil)\"
				eval tmp=\$TCP_NODE$i
				[ -n "$tmp" -a "$tmp" != "nil" ] && {
					test_auto_switch TCP tcp $i "$tmp"
				}
			done
			delay=$(config_t_get auto_switch testing_time 1)
			sleep ${delay}m
		done
	) 200>"/var/lock/passwall_auto_switch.lock"
}

case $1 in
test_url)
	test_url $2
	;;
*)
	start
	;;
esac
