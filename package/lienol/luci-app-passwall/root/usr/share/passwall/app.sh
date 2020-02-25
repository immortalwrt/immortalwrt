#!/bin/sh
# Copyright (C) 2018-2020 Lienol <lawlienol@gmail.com>

. $IPKG_INSTROOT/lib/functions.sh
. $IPKG_INSTROOT/lib/functions/service.sh

CONFIG=passwall
CONFIG_PATH=/var/etc/$CONFIG
RUN_BIN_PATH=$CONFIG_PATH/bin
RUN_ID_PATH=$CONFIG_PATH/id
LOCK_FILE=/var/lock/$CONFIG.lock
LOG_FILE=/var/log/$CONFIG.log
RULE_PATH=/etc/config/${CONFIG}_rule
APP_PATH=/usr/share/$CONFIG
TMP_DNSMASQ_PATH=/var/etc/dnsmasq-passwall.d
DNSMASQ_PATH=/etc/dnsmasq.d
RESOLVFILE=/tmp/resolv.conf.d/resolv.conf.auto
DNS_PORT=7913
LUA_API_PATH=/usr/lib/lua/luci/model/cbi/$CONFIG/api
API_GEN_V2RAY=$LUA_API_PATH/gen_v2ray_client_config.lua
API_GEN_TROJAN=$LUA_API_PATH/gen_trojan_client_config.lua
FWI=$(uci get firewall.passwall.path 2>/dev/null)

echolog() {
	local d="$(date "+%Y-%m-%d %H:%M:%S")"
	echo -e "$d: $1" >>$LOG_FILE
}

find_bin() {
	bin_name=$1
	result=$(find /usr/*bin -iname "$bin_name" -type f)
	if [ -z "$result" ]; then
		echo ""
	else
		echo "$result"
	fi
}

config_n_get() {
	local ret=$(uci -q get $CONFIG.$1.$2 2>/dev/null)
	echo ${ret:=$3}
}

config_t_get() {
	local index=0
	[ -n "$4" ] && index=$4
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	echo ${ret:=$3}
}

get_host_ip() {
	local host=$2
	local isip=""
	local ip=$host
	if [ "$1" == "ipv6" ]; then
		isip=$(echo $host | grep -E "([[a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7}])")
		if [ -n "$isip" ]; then
			isip=$(echo $host | cut -d '[' -f2 | cut -d ']' -f1)
		else
			isip=$(echo $host | grep -E "([a-f0-9]{1,4}(:[a-f0-9]{1,4}){7}|[a-f0-9]{1,4}(:[a-f0-9]{1,4}){0,7}::[a-f0-9]{0,4}(:[a-f0-9]{1,4}){0,7})")
		fi
	else
		isip=$(echo $host | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}")
	fi
	[ -z "$isip" ] && {
		local t=4
		[ "$1" == "ipv6" ] && t=6
		local vpsrip=$(resolveip -$t -t 3 $host | awk 'NR==1{print}')
		ip=$vpsrip
	}
	echo $ip
}

get_node_host_ip() {
	local ip
	local address=$(config_n_get $1 address)
	[ -n "$address" ] && {
		local use_ipv6=$(config_n_get $1 use_ipv6)
		local network_type="ipv4"
		[ "$use_ipv6" == "1" ] && network_type="ipv6"
		ip=$(get_host_ip $network_type $address)
	}
	echo $ip
}

check_port_exists() {
	port=$1
	protocol=$2
	result=
	if [ "$protocol" = "tcp" ]; then
		result=$(netstat -tln | grep -c ":$port")
	elif [ "$protocol" = "udp" ]; then
		result=$(netstat -uln | grep -c ":$port")
	fi
	if [ "$result" = 1 ]; then
		echo 1
	else
		echo 0
	fi
}

get_not_exists_port_after() {
	port=$1
	protocol=$2
	result=$(check_port_exists $port $protocol)
	if [ "$result" = 1 ]; then
		temp=
		if [ "$port" -lt 65535 ]; then
			temp=$(expr $port + 1)
		elif [ "$port" -gt 1 ]; then
			temp=$(expr $port - 1)
		fi
		get_not_exists_port_after $temp $protocol
	else
		echo $port
	fi
}

ln_start_bin() {
	local file=$1
	local bin=$2
	local cmd=$3
	if [ -n "${RUN_BIN_PATH}/$bin" -a -f "${RUN_BIN_PATH}/$bin" ];then
		${RUN_BIN_PATH}/$bin $cmd >/dev/null 2>&1 &
	else
		if [ -n "$file" -a -f "$file" ];then
			ln -s $file ${RUN_BIN_PATH}/$bin
			${RUN_BIN_PATH}/$bin $cmd >/dev/null 2>&1 &
		else
			echolog "找不到$bin主程序，无法启动！"
		fi
	fi
}

ENABLED=$(config_t_get global enabled 0)

TCP_NODE_NUM=$(config_t_get global_other tcp_node_num 1)
for i in $(seq 1 $TCP_NODE_NUM); do
	eval TCP_NODE$i=$(config_t_get global tcp_node$i nil)
done

UDP_NODE_NUM=$(config_t_get global_other udp_node_num 1)
for i in $(seq 1 $UDP_NODE_NUM); do
	eval UDP_NODE$i=$(config_t_get global udp_node$i nil)
done

SOCKS5_NODE_NUM=$(config_t_get global_other socks5_node_num 1)
for i in $(seq 1 $SOCKS5_NODE_NUM); do
	eval SOCKS5_NODE$i=$(config_t_get global socks5_node$i nil)
done

[ "$UDP_NODE1" == "tcp" ] && UDP_NODE1=$TCP_NODE1
[ "$SOCKS5_NODE1" == "tcp" ] && SOCKS5_NODE1=$TCP_NODE1

# Dynamic variables (Used to record)
# TCP_NODE1_IP="" UDP_NODE1_IP="" SOCKS5_NODE1_IP="" TCP_NODE1_PORT="" UDP_NODE1_PORT="" SOCKS5_NODE1_PORT="" TCP_NODE1_TYPE="" UDP_NODE1_TYPE="" SOCKS5_NODE1_TYPE=""

TCP_REDIR_PORTS=$(config_t_get global_forwarding tcp_redir_ports '80,443')
UDP_REDIR_PORTS=$(config_t_get global_forwarding udp_redir_ports '1:65535')
TCP_NO_REDIR_PORTS=$(config_t_get global_forwarding tcp_no_redir_ports 'disable')
UDP_NO_REDIR_PORTS=$(config_t_get global_forwarding udp_no_redir_ports 'disable')
KCPTUN_REDIR_PORT=$(config_t_get global_forwarding kcptun_port 12948)
PROXY_MODE=$(config_t_get global proxy_mode chnroute)

load_config() {
	[ "$ENABLED" != 1 ] && return 1
	[ "$TCP_NODE1" == "nil" -a "$UDP_NODE1" == "nil" -a "$SOCKS5_NODE1" == "nil" ] && {
		echolog "没有选择节点！"
		return 1
	}
	
	DNS_MODE=$(config_t_get global dns_mode pdnsd)
	DNS_FORWARD=$(config_t_get global dns_forward 8.8.4.4)
	use_tcp_node_resolve_dns=$(config_t_get global use_tcp_node_resolve_dns 0)
	use_udp_node_resolve_dns=0
	process=1
	if [ "$(config_t_get global_forwarding process 0)" = "0" ]; then
		process=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
	else
		process=$(config_t_get global_forwarding process)
	fi
	LOCALHOST_PROXY_MODE=$(config_t_get global localhost_proxy_mode default)
	[ "$LOCALHOST_PROXY_MODE" == "default" ] && LOCALHOST_PROXY_MODE=$PROXY_MODE
	UP_CHINA_DNS=$(config_t_get global up_china_dns dnsbyisp)
	wangejibadns=$(config_t_get global_other wangejibadns 0)
	[ "$wangejibadns" == "0" ] && {
		UP_CHINA_DNS="default"
		[ "$DNS_MODE" == "chinadns-ng" ] && DNS_MODE="pdnsd" && use_udp_node_resolve_dns=0
	}
	[ "$UP_CHINA_DNS" == "default" ] && IS_DEFAULT_CHINA_DNS=1
	[ ! -f "$RESOLVFILE" -o ! -s "$RESOLVFILE" ] && RESOLVFILE=/tmp/resolv.conf.auto
	[ "$UP_CHINA_DNS" == "dnsbyisp" -o "$UP_CHINA_DNS" == "default" ] && {
		local dns1=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | grep -v 0.0.0.0 | grep -v 127.0.0.1 | sed -n '1P')
		if [ -n "$dns1" ]; then
			UP_CHINA_DNS=$dns1
		else
			UP_CHINA_DNS="223.5.5.5"
		fi
		local dns2=$(cat $RESOLVFILE 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | grep -v 0.0.0.0 | grep -v 127.0.0.1 | sed -n '2P')
		[ -n "$dns1" -a -n "$dns2" ] && UP_CHINA_DNS="$dns1,$dns2"
	}
	TCP_REDIR_PORT1=$(config_t_get global_forwarding tcp_redir_port 1041)
	TCP_REDIR_PORT2=$(expr $TCP_REDIR_PORT1 + 1)
	TCP_REDIR_PORT3=$(expr $TCP_REDIR_PORT2 + 1)
	UDP_REDIR_PORT1=$(config_t_get global_forwarding udp_redir_port 1051)
	UDP_REDIR_PORT2=$(expr $UDP_REDIR_PORT1 + 1)
	UDP_REDIR_PORT3=$(expr $UDP_REDIR_PORT2 + 1)
	SOCKS5_PROXY_PORT1=$(config_t_get global_forwarding socks5_proxy_port 1081)
	SOCKS5_PROXY_PORT2=$(expr $SOCKS5_PROXY_PORT1 + 1)
	SOCKS5_PROXY_PORT3=$(expr $SOCKS5_PROXY_PORT2 + 1)
	PROXY_IPV6=$(config_t_get global_forwarding proxy_ipv6 0)
	mkdir -p /var/etc $CONFIG_PATH $RUN_BIN_PATH $RUN_ID_PATH
	return 0
}

gen_ss_ssr_config_file() {
	local type local_port kcptun node configfile
	type=$1
	local_port=$2
	kcptun=$3
	node=$4
	configfile=$5
	local port encrypt_method
	port=$(config_n_get $node port)
	encrypt_method=$(config_n_get $node ss_encrypt_method)
	[ "$type" == "ssr" ] && encrypt_method=$(config_n_get $node ssr_encrypt_method)
	[ "$kcptun" == "1" ] && {
		server_host=127.0.0.1
		port=$KCPTUN_REDIR_PORT
	}
	cat <<-EOF >$configfile
		{
		    "server": "$server_host",
		    "server_port": $port,
		    "local_address": "0.0.0.0",
		    "local_port": $local_port,
		    "password": "$(config_n_get $node password)",
		    "timeout": $(config_n_get $node timeout),
		    "method": "$encrypt_method",
		    "fast_open": $(config_n_get $node tcp_fast_open false),
		    "reuse_port": true,
	EOF
	[ "$1" == "ssr" ] && {
		cat <<-EOF >>$configfile
		    "protocol": "$(config_n_get $node protocol)",
		    "protocol_param": "$(config_n_get $node protocol_param)",
		    "obfs": "$(config_n_get $node obfs)",
		    "obfs_param": "$(config_n_get $node obfs_param)"
		EOF
	}
	echo -e "}" >>$configfile
}

gen_start_config() {
	local node local_port redir_type config_file server_host port type
	node=$1
	local_port=$2
	redir_type=$3
	config_file=$4
	type=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
	remarks=$(config_n_get $node remarks)
	server_host=$(config_n_get $node address)
	port=$(config_n_get $node port)
	[ -n "$server_host" -a -n "$port" ] && echolog "$redir_type节点：$remarks，节点：${server_host}:${port}，监听端口：$local_port"

	if [ "$redir_type" == "SOCKS5" ]; then
		eval SOCKS5_NODE${5}_PORT=$port
		if [ "$type" == "socks5" ]; then
			echolog "Socks5节点不能使用Socks5代理节点！"
		elif [ "$type" == "v2ray" -o "$type" == "v2ray_balancing" -o "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY $node nil nil $local_port >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			lua $API_GEN_TROJAN $node client "0.0.0.0" $local_port >$config_file
			ln_start_bin $(find_bin trojan) trojan "-c $config_file"
		elif [ "$type" == "brook" ]; then
			ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook "client -l 0.0.0.0:$local_port -i 0.0.0.0 -s $server_host:$port -p $(config_n_get $node password)"
		elif [ "$type" == "ssr" ]; then
			gen_ss_ssr_config_file ssr $local_port 0 $node $config_file
			ln_start_bin $(find_bin ssr-local) ssr-local "-c $config_file -b 0.0.0.0 -u"
		elif [ "$type" == "ss" ]; then
			gen_ss_ssr_config_file ss $local_port 0 $node $config_file
			local plugin_params=""
			local plugin=$(config_n_get $node ss_plugin)
			if [ "$plugin" != "none" ]; then
				[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
					local opts=$(config_n_get $node ss_plugin_opts)
					plugin_params="--plugin $plugin --plugin-opts $opts"
				}
			fi
			ln_start_bin $(find_bin ss-local) ss-local "-c $config_file -b 0.0.0.0 -u $plugin_params" 
		fi
	fi

	if [ "$redir_type" == "UDP" ]; then
		eval UDP_NODE${5}_PORT=$port
		
		if [ "$type" == "socks5" ]; then
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$UDP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks "-U -l $port -b 0.0.0.0 -s $node_address -p $node_port -R"
			
			# local redsocks_config_file=$CONFIG_PATH/UDP_$i.conf
			# gen_redsocks_config $redsocks_config_file udp $port $node_address $node_port $server_username $server_password
			# ln_start_bin $(find_bin redsocks2) redsocks2 "-c $redsocks_config_file"
		elif [ "$type" == "v2ray" -o "$type" == "v2ray_balancing" -o "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY $node udp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			SOCKS5_PROXY_PORT4=$(expr $SOCKS5_PROXY_PORT3 + 1)
			local_port=$(get_not_exists_port_after $SOCKS5_PROXY_PORT4 tcp)
			socks5_port=$local_port
			lua $API_GEN_TROJAN $node client "127.0.0.1" $socks5_port >$config_file
			ln_start_bin $(find_bin trojan) trojan "-c $config_file"
			
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$UDP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks "-U -l $port -b 0.0.0.0 -s 127.0.0.1 -p $socks5_port -R"
				
			# local redsocks_config_file=$CONFIG_PATH/redsocks_UDP_$i.conf
			# gen_redsocks_config $redsocks_config_file udp $port "127.0.0.1" $socks5_port
			# ln_start_bin $(find_bin redsocks2) redsocks2 "-c $redsocks_config_file"
		elif [ "$type" == "brook" ]; then
			ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook "tproxy -l 0.0.0.0:$local_port -s $server_host:$port -p $(config_n_get $node password)"
		elif [ "$type" == "ssr" ]; then
			gen_ss_ssr_config_file ssr $local_port 0 $node $config_file
			ln_start_bin $(find_bin ssr-redir) ssr-redir "-c $config_file -U"
		elif [ "$type" == "ss" ]; then
			gen_ss_ssr_config_file ss $local_port 0 $node $config_file
			local plugin_params=""
			local plugin=$(config_n_get $node ss_plugin)
			if [ "$plugin" != "none" ]; then
				[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
					local opts=$(config_n_get $node ss_plugin_opts)
					plugin_params="--plugin $plugin --plugin-opts $opts"
				}
			fi
			ln_start_bin $(find_bin ss-redir) ss-redir "-c $config_file -U $plugin_params"
		fi
	fi

	if [ "$redir_type" == "TCP" ]; then
		eval TCP_NODE${5}_PORT=$port
		
		if [ "$type" == "socks5" ]; then
			local node_address=$(config_n_get $node address)
			local node_port=$(config_n_get $node port)
			local server_username=$(config_n_get $node username)
			local server_password=$(config_n_get $node password)
			eval port=\$TCP_REDIR_PORT$5
			ln_start_bin $(find_bin ipt2socks) ipt2socks "-T -l $port -b 0.0.0.0 -s $node_address -p $node_port -R"
			
			# local redsocks_config_file=$CONFIG_PATH/TCP_$i.conf
			# gen_redsocks_config $redsocks_config_file tcp $port $node_address $socks5_port $server_username $server_password
			# ln_start_bin $(find_bin redsocks2) redsocks2 "-c $redsocks_config_file"
		elif [ "$type" == "v2ray" -o "$type" == "v2ray_balancing" -o "$type" == "v2ray_shunt" ]; then
			lua $API_GEN_V2RAY $node tcp $local_port nil >$config_file
			ln_start_bin $(config_t_get global_app v2ray_file $(find_bin v2ray))/v2ray v2ray "-config=$config_file"
		elif [ "$type" == "trojan" ]; then
			lua $API_GEN_TROJAN $node nat "0.0.0.0" $local_port >$config_file
			ln_start_bin $(find_bin trojan) trojan "-c $config_file"
		else
			local kcptun_use=$(config_n_get $node use_kcp 0)
			if [ "$kcptun_use" == "1" ]; then
				local kcptun_server_host=$(config_n_get $node kcp_server)
				local kcptun_port=$(config_n_get $node kcp_port)
				local kcptun_config="$(config_n_get $node kcp_opts)"
				local lbenabled=$(config_t_get global_haproxy balancing_enable 0)
				if [ -z "$kcptun_port" -o -z "$kcptun_config" ]; then
					echolog "【未配置Kcptun参数】，跳过~"
					force_stop
				fi
				if [ -n "$kcptun_port" -a -n "$kcptun_config" -a "$lbenabled" == "0" ]; then
					local run_kcptun_ip=$server_host
					[ -n "$kcptun_server_host" ] && run_kcptun_ip=$(get_host_ip $network_type $kcptun_server_host)
					KCPTUN_REDIR_PORT=$(get_not_exists_port_after $KCPTUN_REDIR_PORT udp)
					ln_start_bin $(config_t_get global_app kcptun_client_file $(find_bin kcptun-client)) kcptun-client "--log $CONFIG_PATH/kcptun_${5}.log -l 0.0.0.0:$KCPTUN_REDIR_PORT -r $run_kcptun_ip:$kcptun_port $kcptun_config"
				fi
			fi
			if [ "$type" == "ssr" ]; then
				gen_ss_ssr_config_file ssr $local_port $kcptun_use $node $config_file
				for k in $(seq 1 $process); do
					ln_start_bin $(find_bin ssr-redir) ssr-redir "-c $config_file"
				done
			elif [ "$type" == "ss" ]; then
				gen_ss_ssr_config_file ss $local_port $kcptun_use $node $config_file
				local plugin_params=""
				local plugin=$(config_n_get $node ss_plugin)
				if [ "$plugin" != "none" ]; then
					[ "$plugin" == "v2ray-plugin" -o "$plugin" == "obfs-local" ] && {
					local opts=$(config_n_get $node ss_plugin_opts)
					plugin_params="--plugin $plugin --plugin-opts $opts"
					}
				fi
				for k in $(seq 1 $process); do
					ln_start_bin $(find_bin ss-redir) ss-redir "-c $config_file $plugin_params"
				done
			elif [ "$type" == "brook" ]; then
				local server_ip=$server_host
				[ "$kcptun_use" == "1" ] && {
					server_ip=127.0.0.1
					port=$KCPTUN_REDIR_PORT
				}
				ln_start_bin $(config_t_get global_app brook_file $(find_bin brook)) brook "tproxy -l 0.0.0.0:$local_port -s $server_ip:$port -p $(config_n_get $node password)"
			fi
		fi
	fi
	return 0
}

start_redir() {
	eval num=\$${1}_NODE_NUM
	for i in $(seq 1 $num); do
		eval node=\$${1}_NODE$i
		[ "$node" != "nil" ] && {
			TYPE=$(echo $(config_n_get $node type) | tr 'A-Z' 'a-z')
			local config_file=$CONFIG_PATH/${1}_${i}.json
			eval current_port=\$${1}_${2}_PORT$i
			local port=$(echo $(get_not_exists_port_after $current_port $3))
			eval ${1}_${2}$i=$port
			gen_start_config $node $port $1 $config_file $i
			#eval ip=\$${1}_NODE${i}_IP
			echo $node > $RUN_ID_PATH/${1}_${i}
		}
	done
}

clean_log() {
	logsnum=$(cat $LOG_FILE 2>/dev/null | wc -l)
	[ "$logsnum" -gt 300 ] && {
		echo "" > $LOG_FILE
		echolog "日志文件过长，清空处理！"
	}
}

start_crontab() {
	sed -i '/$CONFIG/d' /etc/crontabs/root >/dev/null 2>&1 &
	start_daemon=$(config_t_get global_delay start_daemon)
	if [ "$start_daemon" = "1" ]; then
		echo "*/2 * * * * nohup $APP_PATH/monitor.sh > /dev/null 2>&1" >>/etc/crontabs/root
		echolog "已启动守护进程。"
	fi

	auto_on=$(config_t_get global_delay auto_on 0)
	if [ "$auto_on" = "1" ]; then
		time_off=$(config_t_get global_delay time_off)
		time_on=$(config_t_get global_delay time_on)
		time_restart=$(config_t_get global_delay time_restart)
		[ -z "$time_off" -o "$time_off" != "nil" ] && {
			echo "0 $time_off * * * /etc/init.d/$CONFIG stop" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_off 点关闭服务。"
		}
		[ -z "$time_on" -o "$time_on" != "nil" ] && {
			echo "0 $time_on * * * /etc/init.d/$CONFIG start" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_on 点开启服务。"
		}
		[ -z "$time_restart" -o "$time_restart" != "nil" ] && {
			echo "0 $time_restart * * * /etc/init.d/$CONFIG restart" >>/etc/crontabs/root
			echolog "配置定时任务：每天 $time_restart 点重启服务。"
		}
	fi

	AUTO_SWITCH_ENABLE=$(config_t_get auto_switch enable 0)
	[ "$AUTO_SWITCH_ENABLE" = "1" ] && {
		testing_time=$(config_t_get auto_switch testing_time)
		[ -n "$testing_time" ] && {
			echo "*/$testing_time * * * * nohup $APP_PATH/test.sh > /dev/null 2>&1" >>/etc/crontabs/root
			echolog "配置定时任务：每$testing_time分钟执行自动切换检测脚本。"
		}
	}
	
	autoupdate=$(config_t_get global_rules auto_update)
	weekupdate=$(config_t_get global_rules week_update)
	dayupdate=$(config_t_get global_rules time_update)
	autoupdatesubscribe=$(config_t_get global_subscribe auto_update_subscribe)
	weekupdatesubscribe=$(config_t_get global_subscribe week_update_subscribe)
	dayupdatesubscribe=$(config_t_get global_subscribe time_update_subscribe)
	if [ "$autoupdate" = "1" ]; then
		local t="0 $dayupdate * * $weekupdate"
		[ "$weekupdate" = "7" ] && t="0 $dayupdate * * *"
		echo "$t lua $APP_PATH/rule_update.lua nil log > /dev/null 2>&1 &" >>/etc/crontabs/root
		echolog "配置定时任务：自动更新规则。"
	fi

	if [ "$autoupdatesubscribe" = "1" ]; then
		local t="0 $dayupdatesubscribe * * $weekupdate"
		[ "$weekupdatesubscribe" = "7" ] && t="0 $dayupdatesubscribe * * *"
		echo "$t lua $APP_PATH/subscribe.lua start log > /dev/null 2>&1 &" >>/etc/crontabs/root
		echolog "配置定时任务：自动更新节点订阅。"
	fi
	
	/etc/init.d/cron restart
}

stop_crontab() {
	sed -i "/$CONFIG/d" /etc/crontabs/root >/dev/null 2>&1 &
	ps | grep "$APP_PATH/test.sh" | grep -v "grep" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	rm -f /var/lock/${CONFIG}_test.lock >/dev/null 2>&1 &
	/etc/init.d/cron restart
	echolog "清除定时执行命令。"
}

start_dns() {
	case "$DNS_MODE" in
	nonuse)
		echolog "DNS：不使用，将会直接使用上级DNS！"
	;;
	local_7913)
		echolog "DNS：使用本机7913端口DNS服务器解析域名..."
	;;
	dns2socks)
		if [ -n "$SOCKS5_NODE1" -a "$SOCKS5_NODE1" != "nil" ]; then
			DNS2SOCKS_FORWARD=$(config_t_get global dns2socks_forward 8.8.4.4)
			ln_start_bin $(find_bin dns2socks) dns2socks "127.0.0.1:$SOCKS5_PROXY_PORT1 $DNS2SOCKS_FORWARD 127.0.0.1:$DNS_PORT"
			echolog "DNS：dns2socks..."
		else
			echolog "DNS：dns2socks模式需要使用Socks5代理节点，请开启！"
			force_stop
		fi
	;;
	pdnsd)
		use_tcp_node_resolve_dns=1
		gen_pdnsd_config $DNS_PORT 10240
		DNS_FORWARD=$(echo $DNS_FORWARD | sed 's/,/ /g')
		ln_start_bin $(find_bin pdnsd) pdnsd "--daemon -c $pdnsd_dir/pdnsd.conf -d"
		echolog "DNS：pdnsd..."
	;;
	chinadns-ng)
		other_port=$(expr $DNS_PORT + 1)
		cat $RULE_PATH/gfwlist.conf | sort | uniq | sed -e '/127.0.0.1/d' | sed 's/ipset=\/.//g' | sed 's/\/gfwlist//g' > $CONFIG_PATH/gfwlist.txt
		[ -f "$CONFIG_PATH/gfwlist.txt" ] && local gfwlist_param="-g $CONFIG_PATH/gfwlist.txt"
		[ -f "$RULE_PATH/chnlist" ] && local chnlist_param="-m $RULE_PATH/chnlist"
		
		up_trust_chinadns_ng_dns=$(config_t_get global up_trust_chinadns_ng_dns "pdnsd")
		if [ "$up_trust_chinadns_ng_dns" == "pdnsd" ]; then
			if [ -z "$TCP_NODE1" -o "$TCP_NODE1" == "nil" ]; then
				echolog "DNS：ChinaDNS-NG + pdnsd 模式需要启用TCP节点！"
				force_stop
			else
				use_tcp_node_resolve_dns=1
				gen_pdnsd_config $other_port 0
				DNS_FORWARD=$(echo $DNS_FORWARD | sed 's/,/ /g')
				ln_start_bin $(find_bin pdnsd) pdnsd "--daemon -c $pdnsd_dir/pdnsd.conf -d"
				ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t 127.0.0.1#$other_port $gfwlist_param $chnlist_param"
				echolog "DNS：ChinaDNS-NG + pdnsd($DNS_FORWARD)，国内DNS：$UP_CHINA_DNS"
			fi
		elif [ "$up_trust_chinadns_ng_dns" == "dns2socks" ]; then
			if [ -n "$SOCKS5_NODE1" -a "$SOCKS5_NODE1" != "nil" ]; then
				DNS2SOCKS_FORWARD=$(config_t_get global dns2socks_forward 8.8.4.4)
				ln_start_bin $(find_bin dns2socks) dns2socks "127.0.0.1:$SOCKS5_PROXY_PORT1 $DNS2SOCKS_FORWARD 127.0.0.1:$other_port"
				ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t 127.0.0.1#$other_port $gfwlist_param $chnlist_param"
				echolog "DNS：ChinaDNS-NG + dns2socks($DNS2SOCKS_FORWARD)，国内DNS：$UP_CHINA_DNS"
			else
				echolog "DNS：dns2socks模式需要使用Socks5代理节点，请开启！"
				force_stop
			fi
		else
			use_udp_node_resolve_dns=1
			DNS_FORWARD=$(echo $up_trust_chinadns_ng_dns | sed 's/,/ /g')
			ln_start_bin $(find_bin chinadns-ng) chinadns-ng "-l $DNS_PORT -c $UP_CHINA_DNS -t $up_trust_chinadns_ng_dns $gfwlist_param $chnlist_param"
			echolog "DNS：ChinaDNS-NG，国内DNS：$UP_CHINA_DNS，可信DNS：$up_trust_chinadns_ng_dns，如果不能使用，请确保UDP节点已打开并且支持UDP转发。"
		fi
	;;
	esac
}

add_dnsmasq() {
	mkdir -p $TMP_DNSMASQ_PATH $DNSMASQ_PATH /var/dnsmasq.d
	cat $RULE_PATH/whitelist_host | sed -e "/^$/d" | sed "s/^/ipset=&\/./g" | sed "s/$/\/&whitelist/g" | sort | awk '{if ($0!=line) print;line=$0}' > $TMP_DNSMASQ_PATH/whitelist_host.conf

	local adblock=$(config_t_get global_rules adblock 1)
	[ "$adblock" == "1" ] && {
		[ -f "$RULE_PATH/adblock.conf" -a -s "$RULE_PATH/adblock.conf" ] && ln -s $RULE_PATH/adblock.conf $TMP_DNSMASQ_PATH/adblock.conf
	}

	[ "$DNS_MODE" != "nonuse" ] && {
		[ -f "$RULE_PATH/blacklist_host" -a -s "$RULE_PATH/blacklist_host" ] && cat $RULE_PATH/blacklist_host | sed -e "/^$/d" | awk '{print "server=/."$1"/127.0.0.1#'$DNS_PORT'\nipset=/."$1"/blacklist"}' > $TMP_DNSMASQ_PATH/blacklist_host.conf
		[ -f "$RULE_PATH/router" -a -s "$RULE_PATH/router" ] && cat $RULE_PATH/router | sed -e "/^$/d" | awk '{print "server=/."$1"/127.0.0.1#'$DNS_PORT'\nipset=/."$1"/router"}' > $TMP_DNSMASQ_PATH/router.conf
		[ -f "$RULE_PATH/gfwlist.conf" -a -s "$RULE_PATH/gfwlist.conf" ] && ln -s $RULE_PATH/gfwlist.conf $TMP_DNSMASQ_PATH/gfwlist.conf
		
		subscribe_proxy=$(config_t_get global_subscribe subscribe_proxy 0)
		[ "$subscribe_proxy" -eq 1 ] && {
			local count=$(uci show $CONFIG | grep "@subscribe_list" | sed -n '$p' | cut -d '[' -f 2 | cut -d ']' -f 1)
			[ -n "$count" -a "$count" -ge 0 ] && {
				u_get() {
					local ret=$(uci -q get $CONFIG.@subscribe_list[$1].$2)
					echo ${ret:=$3}
				}
				for i in $(seq 0 $count); do
					local enabled=$(u_get $i enabled 0)
					[ "$enabled" == "0" ] && continue
					local url=$(u_get $i url)
					[ -n "$url" -a "$url" != "" ] && {
						if [ -n "$(echo -n "$url" | grep "//")" ]; then
							echo -n "$url" | awk -F'/' '{print $3}' | sed "s/^/server=&\/./g" | sed "s/$/\/127.0.0.1#$DNS_PORT/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
							echo -n "$url" | awk -F'/' '{print $3}' | sed "s/^/ipset=&\/./g" | sed "s/$/\/router/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
						else
							echo -n "$url" | awk -F'/' '{print $1}' | sed "s/^/server=&\/./g" | sed "s/$/\/127.0.0.1#$DNS_PORT/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
							echo -n "$url" | awk -F'/' '{print $1}' | sed "s/^/ipset=&\/./g" | sed "s/$/\/router/g" >>$TMP_DNSMASQ_PATH/subscribe.conf
						fi
					}
				done
			}
		}
	}
	
	[ -z "$IS_DEFAULT_CHINA_DNS" -o "$IS_DEFAULT_CHINA_DNS" == 0 ] && {
		server="server=127.0.0.1#$DNS_PORT"
		[ "$DNS_MODE" != "chinadns-ng" ] && {
			local china_dns1=$(echo $UP_CHINA_DNS | awk -F "," '{print $1}')
			local china_dns2=$(echo $UP_CHINA_DNS | awk -F "," '{print $2}')
			[ -n "$china_dns1" ] && server="server=$china_dns1"
			[ -n "$china_dns2" ] && server="${server}\n${server_2}"
			server="${server}\nno-resolv"
		}
		cat <<-EOF > /var/dnsmasq.d/dnsmasq-$CONFIG.conf
			$(echo -e $server)
			all-servers
			no-poll
		EOF
	}
	
	echo "conf-dir=$TMP_DNSMASQ_PATH" >> /var/dnsmasq.d/dnsmasq-$CONFIG.conf
	cp -rf /var/dnsmasq.d/dnsmasq-$CONFIG.conf $DNSMASQ_PATH/dnsmasq-$CONFIG.conf
	echolog "dnsmasq：生成配置文件。"
}

gen_redsocks_config() {
	protocol=$2
	local_port=$3
	proxy_server=$4
	proxy_port=$5
	proxy_username=$6
	[ -n "$proxy_username" ] && proxy_username="login = $proxy_username;"
	proxy_password=$7
	[ -n "$proxy_password" ] && proxy_password="password = $proxy_password;"
	[ -n "$1" ] && {
		cat >$1 <<-EOF
			base {
			    log_debug = off;
			    log_info = off;
			    log = "file:/dev/null";
			    daemon = on;
			    redirector = iptables;
			}
			
		EOF
		if [ "$protocol" == "tcp" ]; then
			cat >>$1 <<-EOF
				redsocks {
				    local_ip = 0.0.0.0;
				    local_port = $local_port;
				    type = socks5;
				    autoproxy = 0;
				    ip = $proxy_server;
				    port = $proxy_port;
				    $proxy_username
				    $proxy_password
				}
				
				autoproxy {
				    no_quick_check_seconds = 300;
				    quick_connect_timeout = 2;
				}
				
				ipcache {
				    cache_size = 4;
				    stale_time = 7200;
				    autosave_interval = 3600;
				    port_check = 0;
				}
				
			EOF
		elif [ "$protocol" == "udp" ]; then
			cat >>$1 <<-EOF
				redudp {
				    local_ip = 0.0.0.0;
				    local_port = $local_port;
				    type = socks5;
				    ip = $proxy_server;
				    port = $proxy_port;
				    $proxy_username
				    $proxy_password
				    udp_timeout = 60;
				    udp_timeout_stream = 360;
				}
				
			EOF
		fi
	}
}

gen_pdnsd_config() {
	pdnsd_dir=$CONFIG_PATH/pdnsd
	mkdir -p $pdnsd_dir
	touch $pdnsd_dir/pdnsd.cache
	chown -R root.nogroup $pdnsd_dir
	cat > $pdnsd_dir/pdnsd.conf <<-EOF
		global {
			perm_cache = $2;
			cache_dir = "$pdnsd_dir";
			run_as = "root";
			server_ip = 127.0.0.1;
			server_port = $1;
			status_ctl = on;
			query_method = tcp_only;
			min_ttl = 1d;
			max_ttl = 1w;
			timeout = 10;
			tcp_qtimeout = 1;
			par_queries = 1;
			neg_domain_pol = on;
			udpbufsize = 1024;
		}
		
	EOF
			
	[ "$use_tcp_node_resolve_dns" == 1 ] && {
		cat >> $pdnsd_dir/pdnsd.conf <<-EOF
			server {
				label = "node";
				ip = $DNS_FORWARD;
				edns_query = on;
				port = 53;
				timeout = 4;
				interval = 60;
				uptest = none;
				purge_cache = off;
			}
			
		EOF
	}
		
	cat >> $pdnsd_dir/pdnsd.conf <<-EOF
		server {
			label = "opendns";
			ip = 208.67.222.222, 208.67.220.220;
			edns_query = on;
			port = 443;
			timeout = 4;
			interval = 60;
			uptest = none;
			purge_cache = off;
		}
		server {
			label = "opendns";
			ip = 208.67.222.222, 208.67.220.220;
			edns_query = on;
			port = 5353;
			timeout = 4;
			interval = 60;
			uptest = none;
			purge_cache = off;
		}
		source {
			ttl = 86400;
			owner = "localhost.";
			serve_aliases = on;
			file = "/etc/hosts";
		}
	EOF
}

stop_dnsmasq() {
	rm -rf /var/dnsmasq.d/dnsmasq-$CONFIG.conf
	rm -rf $DNSMASQ_PATH/dnsmasq-$CONFIG.conf
	rm -rf $TMP_DNSMASQ_PATH
	/etc/init.d/dnsmasq restart >/dev/null 2>&1 &
}

start_haproxy() {
	enabled=$(config_t_get global_haproxy balancing_enable 0)
	[ "$enabled" = "1" ] && {
		haproxy_bin=$(find_bin haproxy)
		[ -f "$haproxy_bin" ] && {
			local HAPROXY_PATH=$CONFIG_PATH/haproxy
			mkdir -p $HAPROXY_PATH
			local HAPROXY_FILE=$HAPROXY_PATH/config.cfg
			bport=$(config_t_get global_haproxy haproxy_port)
			cat <<-EOF >$HAPROXY_FILE
				global
				    log         127.0.0.1 local2
				    chroot      /usr/bin
				    maxconn     60000
				    stats socket  $HAPROXY_PATH/haproxy.sock
				    user        root
				    daemon
					
				defaults
				    mode                    tcp
				    log                     global
				    option                  tcplog
				    option                  dontlognull
				    option http-server-close
				    #option forwardfor       except 127.0.0.0/8
				    option                  redispatch
				    retries                 2
				    timeout http-request    10s
				    timeout queue           1m
				    timeout connect         10s
				    timeout client          1m
				    timeout server          1m
				    timeout http-keep-alive 10s
				    timeout check           10s
				    maxconn                 3000
					
				listen passwall
				    bind 0.0.0.0:$bport
				    mode tcp
			EOF
			local count=$(uci show $CONFIG | grep "@balancing" | sed -n '$p' | cut -d '[' -f 2 | cut -d ']' -f 1)
			[ -n "$count" -a "$count" -ge 0 ] && {
				u_get() {
					local ret=$(uci -q get $CONFIG.@balancing[$1].$2)
					echo ${ret:=$3}
				}
				for i in $(seq 0 $count); do
					enabled=$(u_get $i enabled 0)
					[ "$enabled" == "0" ] && continue
					bips=$(u_get $i lbss)
					bports=$(u_get $i lbort)
					if [ -z "$bips" ] || [ -z "$bports" ]; then
						break
					fi
					local bip=$(echo $bips | awk -F ":" '{print $1}')
					local bport=$(echo $bips | awk -F ":" '{print $2}')
					[ "$bports" != "default" ] && bport=$bports
					[ -z "$bport" ] && break
					
					bweight=$(u_get $i lbweight)
					exports=$(u_get $i export)
					bbackup=$(u_get $i backup)
					if [ "$bbackup" = "1" ]; then
						bbackup=" backup"
						echolog "负载均衡：添加故障转移备节点:$bip"
					else
						bbackup=""
						echolog "负载均衡：添加负载均衡主节点:$bip"
					fi
					#si=$(echo $bip | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}")
					#if [ -z "$si" ]; then
					#	bip=$(resolveip -4 -t 2 $bip | awk 'NR==1{print}')
					#	if [ -z "$bip" ]; then
					#		bip=$(nslookup $bip localhost | sed '1,4d' | awk '{print $3}' | grep -v : | awk 'NR==1{print}')
					#	fi
					#	echolog "负载均衡${i} IP为：$bip"
					#fi
					echo "    server $bip:$bport $bip:$bport weight $bweight check inter 1500 rise 1 fall 3 $bbackup" >> $HAPROXY_FILE
					if [ "$exports" != "0" ]; then
						failcount=0
						while [ "$failcount" -lt "3" ]; do
							interface=$(ifconfig | grep "$exports" | awk '{print $1}')
							if [ -z "$interface" ]; then
								echolog "找不到出口接口：$exports，1分钟后再重试"
								let "failcount++"
								[ "$failcount" -ge 3 ] && exit 0
								sleep 1m
							else
								route add -host ${bip} dev ${exports}
								echo "$bip" >>/tmp/balancing_ip
								break
							fi
						done
					fi
				done
			}
			#生成负载均衡控制台
			console_port=$(config_t_get global_haproxy console_port)
			console_user=$(config_t_get global_haproxy console_user)
			console_password=$(config_t_get global_haproxy console_password)
			local auth=""
			[ -n "$console_user" -a -n "console_password" ] && auth="stats auth $console_user:$console_password"
			cat <<-EOF >> $HAPROXY_FILE
			
				listen status
				    bind 0.0.0.0:$console_port
				    mode http                   
				    stats refresh 30s
				    stats uri /
				    stats admin if TRUE
					$auth
			EOF
			ln_start_bin $haproxy_bin haproxy "-f $HAPROXY_FILE"
		}
	}
}

flush_include() {
	echo '#!/bin/sh' >$FWI
}

gen_include() {
	flush_include
	extract_rules() {
		echo "*$1"
		iptables-save -t $1 | grep PSW | \
		sed -e "s/^-A \(OUTPUT\|PREROUTING\)/-I \1 1/"
		echo 'COMMIT'
	}
	cat <<-EOF >>$FWI
		iptables-save -c | grep -v "PSW" | iptables-restore -c
		iptables-restore -n <<-EOT
		$(extract_rules nat)
		$(extract_rules mangle)
		EOT
	EOF
	return 0
}

kill_all() {
	kill -9 $(pidof $@) >/dev/null 2>&1 &
}

force_stop() {
	rm -f "$LOCK_FILE"
	exit 0
}

boot() {
	[ "$ENABLED" == 1 ] && {
		local delay=$(config_t_get global_delay start_delay 1)
		if [ "$delay" -gt 0 ]; then
			echolog "执行启动延时 $delay 秒后再启动!"
			sleep $delay && start >/dev/null 2>&1 &
		else
			start
		fi
	}
	return 0
}

start() {
	! load_config && return 1
	[ -f "$LOCK_FILE" ] && return 3
	touch "$LOCK_FILE"
	start_haproxy
	start_redir SOCKS5 PROXY tcp
	start_redir TCP REDIR tcp
	start_redir UDP REDIR udp
	start_dns
	add_dnsmasq
	source $APP_PATH/iptables.sh start
	gen_include
	start_crontab
	/etc/init.d/dnsmasq restart >/dev/null 2>&1 &
	echolog "运行完成！\n"
	rm -f "$LOCK_FILE"
	return 0
}

stop() {
	failcount=1
	while [ "$failcount" -le 10 ]; do
		if [ -f "$LOCK_FILE" ]; then
			let "failcount++"
			sleep 1s
			[ "$failcount" -ge 10 ] && rm -f "$LOCK_FILE"
		else
			break
		fi
	done
	clean_log
	source $APP_PATH/iptables.sh stop
	flush_include
	kill_all v2ray-plugin obfs-local
	ps -w | grep -E "$CONFIG_PATH" | grep -v "grep" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1 &
	rm -rf $TMP_DNSMASQ_PATH $CONFIG_PATH
	stop_dnsmasq
	stop_crontab
	echolog "关闭相关程序，清理相关文件和缓存完成。"
}

case $1 in
stop)
	[ -n "$2" -a "$2" == "force" ] && force_stop
	stop
	;;
start)
	start
	;;
boot)
	boot
	;;
*)
	echo "Usage: $0 (start|stop|restart)"
	;;
esac
