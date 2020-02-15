#!/bin/sh

CONFIG=passwall
CONFIG_PATH=/var/etc/$CONFIG
RUN_BIN_PATH=$CONFIG_PATH/bin

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

dns_mode=$(config_t_get global dns_mode)
use_haproxy=$(config_t_get global_haproxy balancing_enable 0)

#tcp
for i in $(seq 1 $TCP_NODE_NUM); do
	eval tmp_node=\$TCP_NODE$i
	if [ "$tmp_node" != "nil" ]; then
		#kcptun
		use_kcp=$(config_n_get $tmp_node use_kcp 0)
		if [ $use_kcp -gt 0 ]; then
			icount=$(ps -w | grep -v grep | grep $RUN_BIN_PATH | grep kcptun-client | grep kcptun_${i} | wc -l)
			if [ $icount = 0 ]; then
				/etc/init.d/passwall restart
				exit 0
			fi
		fi
		[ -f "/var/etc/passwall/port/TCP_$i" ] && listen_port=$(echo -n `cat /var/etc/passwall/port/TCP_$i`)
		icount=$(ps -w | grep -v grep | grep $RUN_BIN_PATH | grep -i -E "TCP_${i}|brook tproxy -l 0.0.0.0:${listen_port}|ipt2socks -T -l ${listen_port}" | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/passwall restart
			exit 0
		fi
	fi
done


#udp
for i in $(seq 1 $UDP_NODE_NUM); do
	eval tmp_node=\$UDP_NODE$i
	if [ "$tmp_node" != "nil" ]; then
		[ "$tmp_node" == "default" ] && tmp_node=$TCP_NODE1
		[ -f "/var/etc/passwall/port/UDP_$i" ] && listen_port=$(echo -n `cat /var/etc/passwall/port/UDP_$i`)
		icount=$(ps -w | grep -v grep | grep $RUN_BIN_PATH | grep -i -E "UDP_${i}|brook tproxy -l 0.0.0.0:${listen_port}|ipt2socks -U -l ${listen_port}" | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/passwall restart
			exit 0
		fi
	fi
done

#socks5
for i in $(seq 1 $SOCKS5_NODE_NUM); do
	eval tmp_node=\$SOCKS5_NODE$i
	if [ "$tmp_node" != "nil" ]; then
		[ -f "/var/etc/passwall/port/SOCKS5_$i" ] && listen_port=$(echo -n `cat /var/etc/passwall/port/SOCKS5_$i`)
		icount=$(ps -w | grep -v grep | grep $RUN_BIN_PATH | grep -i -E "SOCKS5_${i}|brook client -l 0.0.0.0:${listen_port}" | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/passwall restart
			exit 0
		fi
	fi
done

#dns
if [ "$dns_mode" != "nonuse" ]; then
	icount=$(netstat -apn | grep 7913 | wc -l)
	if [ $icount = 0 ]; then
		/etc/init.d/passwall restart
		exit 0
	fi
fi

#haproxy
if [ $use_haproxy -gt 0 ]; then
	icount=$(ps -w | grep -v grep | grep $RUN_BIN_PATH | grep haproxy | wc -l)
	if [ $icount = 0 ]; then
		/etc/init.d/passwall restart
		exit 0
	fi
fi
