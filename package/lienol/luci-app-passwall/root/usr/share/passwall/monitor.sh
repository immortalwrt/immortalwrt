#!/bin/sh

CONFIG=passwall
TMP_PATH=/var/etc/$CONFIG
TMP_BIN_PATH=$TMP_PATH/bin
TMP_ID_PATH=$TMP_PATH/id

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

if [ "$(top -bn1 | grep -v grep | grep $CONFIG/monitor.sh | wc -l)" -gt 2 ]; then
	exit 1
fi

ENABLED=$(config_t_get global enabled 0)
[ "$ENABLED" != 1 ] && return 1
ENABLED=$(config_t_get global_delay start_daemon 0)
[ "$ENABLED" != 1 ] && return 1
sleep 58s
while [ "$ENABLED" -eq 1 ]
do
	#TCP
	[ -f "$TMP_ID_PATH/TCP" ] && {
		TCP_NODE=$(cat $TMP_ID_PATH/TCP)
		if [ "$TCP_NODE" != "nil" ]; then
			#kcptun
			use_kcp=$(config_n_get $TCP_NODE use_kcp 0)
			if [ $use_kcp -gt 0 ]; then
				icount=$(top -bn1 | grep -v grep | grep "$TMP_BIN_PATH/kcptun" | grep -i "tcp" | wc -l)
				if [ $icount = 0 ]; then
					/etc/init.d/$CONFIG restart
					exit 0
				fi
			fi
			icount=$(top -bn1 | grep -v -E 'grep|kcptun' | grep "$TMP_BIN_PATH" | grep -i "TCP" | wc -l)
			if [ $icount = 0 ]; then
				/etc/init.d/$CONFIG restart
				exit 0
			fi
		fi
	}

	#udp
	[ -f "$TMP_ID_PATH/UDP" ] && {
		UDP_NODE=$(cat $TMP_ID_PATH/UDP)
		if [ "$UDP_NODE" != "nil" ]; then
			[ "$UDP_NODE" == "tcp" ] && continue
			[ "$UDP_NODE" == "tcp_" ] && UDP_NODE=$TCP_NODE
			icount=$(top -bn1 | grep -v grep | grep "$TMP_BIN_PATH" | grep -i "UDP" | wc -l)
			if [ $icount = 0 ]; then
				/etc/init.d/$CONFIG restart
				exit 0
			fi
		fi
	}

	#dns
	dns_mode=$(config_t_get global dns_mode)
	if [ "$dns_mode" != "nonuse" ] && [ "$dns_mode" != "custom" ] && [ "$dns_mode" != "fake_ip" ]; then
		icount=$(netstat -apn | grep 7913 | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	fi
	
	[ -f "$TMP_BIN_PATH/chinadns-ng" ] && {
		icount=$(top -bn1 | grep -v grep | grep $TMP_BIN_PATH/chinadns-ng | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	}

	#haproxy
	use_haproxy=$(config_t_get global_haproxy balancing_enable 0)
	if [ $use_haproxy -gt 0 ]; then
		icount=$(top -bn1 | grep -v grep | grep "$TMP_BIN_PATH/haproxy" | wc -l)
		if [ $icount = 0 ]; then
			/etc/init.d/$CONFIG restart
			exit 0
		fi
	fi
	
	sleep 58s
done
