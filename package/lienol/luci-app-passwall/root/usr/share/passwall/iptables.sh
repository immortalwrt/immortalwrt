#!/bin/sh

IPSET_LANIPLIST="laniplist"
IPSET_VPSIPLIST="vpsiplist"
IPSET_ROUTER="router"
IPSET_GFW="gfwlist"
IPSET_CHN="chnroute"
IPSET_BLACKLIST="blacklist"
IPSET_WHITELIST="whitelist"

ipt_n="iptables -t nat"
ipt_m="iptables -t mangle"
ip6t_n="ip6tables -t nat"
ipt_comment="-m comment --comment PassWall"

factor() {
	if [ -z "$1" ] || [ -z "$2" ]; then
		echo ""
	else
		echo "$2 $1"
	fi
}

get_jump_mode() {
	case "$1" in
	disable)
		echo "j"
		;;
	*)
		echo "g"
		;;
	esac
}

get_ip_mark() {
	if [ -z "$1" ]; then
		echo ""
	else
		echo $1 | awk -F "." '{printf ("0x%02X", $1)} {printf ("%02X", $2)} {printf ("%02X", $3)} {printf ("%02X", $4)}'
	fi
}

dst() {
	echo "-m set --match-set $1 dst"
}


get_action_chain() {
	case "$1" in
	disable)
		echo "RETURN"
		;;
	global)
		echo "PSW_GLO"
		;;
	gfwlist)
		echo "PSW_GFW"
		;;
	chnroute)
		echo "PSW_CHN"
		;;
	gamemode)
		echo "PSW_GAME"
		;;
	returnhome)
		echo "PSW_HOME"
		;;
	esac
}

get_action_chain_name() {
	case "$1" in
	disable)
		echo "不代理"
		;;
	global)
		echo "全局"
		;;
	gfwlist)
		echo "GFW"
		;;
	chnroute)
		echo "大陆白名单"
		;;
	gamemode)
		echo "游戏"
		;;
	returnhome)
		echo "回国"
		;;
	esac
}

gen_laniplist() {
	cat <<-EOF
		0.0.0.0/8
		10.0.0.0/8
		100.64.0.0/10
		127.0.0.0/8
		169.254.0.0/16
		172.16.0.0/12
		192.168.0.0/16
		224.0.0.0/4
		240.0.0.0/4
	EOF
}

load_acl() {
	local enabled
	local remarks
	local ip
	local mac
	local proxy_mode
	local tcp_node
	local udp_node
	local tcp_redir_ports
	local udp_redir_ports
	config_get enabled $1 enabled
	config_get remarks $1 remarks
	config_get ip $1 ip
	config_get mac $1 mac
	config_get proxy_mode $1 proxy_mode
	config_get tcp_node $1 tcp_node
	config_get udp_node $1 udp_node
	config_get tcp_no_redir_ports $1 tcp_no_redir_ports
	config_get udp_no_redir_ports $1 udp_no_redir_ports
	config_get tcp_redir_ports $1 tcp_redir_ports
	config_get udp_redir_ports $1 udp_redir_ports
	[ -z "$proxy_mode" -o "$proxy_mode" = "default" ] && proxy_mode=$PROXY_MODE
	[ -z "$tcp_no_redir_ports" -o "$tcp_no_redir_ports" = "default" ] && tcp_no_redir_ports=$TCP_NO_REDIR_PORTS
	[ -z "$udp_no_redir_ports" -o "$udp_no_redir_ports" = "default" ] && udp_no_redir_ports=$UDP_NO_REDIR_PORTS
	[ -z "$tcp_redir_ports" -o "$tcp_redir_ports" = "default" ] && tcp_redir_ports=$TCP_REDIR_PORTS
	[ -z "$udp_redir_ports" -o "$udp_redir_ports" = "default" ] && udp_redir_ports=$UDP_REDIR_PORTS
	[ -z "$tcp_node" -o "$TCP_NODE_NUM" == "1" ] && tcp_node=1
	[ -z "$udp_node" -o "$UDP_NODE_NUM" == "1" ] && udp_node=1
	eval TCP_NODE=\$TCP_NODE$tcp_node
	eval UDP_NODE=\$UDP_NODE$udp_node
	local ip_mark=$(get_ip_mark $ip)
	[ "$enabled" == "1" -a -n "$proxy_mode" ] && {
		if [ -n "$ip" ] || [ -n "$mac" ]; then
			if [ -n "$ip" -a -n "$mac" ]; then
				echolog "访问控制：IP：$ip，MAC：$mac，代理模式：$(get_action_chain_name $proxy_mode)"
			else
				[ -n "$ip" ] && echolog "访问控制：IP：$ip，代理模式：$(get_action_chain_name $proxy_mode)"
				[ -n "$mac" ] && echolog "访问控制：MAC：$mac，代理模式：$(get_action_chain_name $proxy_mode)"
			fi
			
			if [ "$proxy_mode" == "disable" ]; then
				$ipt_n -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m comment --comment "$remarks" -j RETURN
				$ipt_m -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m comment --comment "$remarks" -j RETURN
			else
				[ "$TCP_NODE" != "nil" ] && {
					[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_n -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
					eval tcp_redir_port=\$TCP_REDIR_PORT$tcp_node
					$ipt_n -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp $(dst $IPSET_BLACKLIST) -m comment --comment "$remarks" -j REDIRECT --to-ports $tcp_redir_port
					$ipt_n -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p tcp $(factor $tcp_redir_ports "-m multiport --dport") -m comment --comment "$remarks" -$(get_jump_mode $proxy_mode) $(get_action_chain $proxy_mode)$tcp_node
				}
				[ "$UDP_NODE" != "nil" ] && {
					[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
					eval udp_redir_port=\$UDP_REDIR_PORT$udp_node
					$ipt_m -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp $(dst $IPSET_BLACKLIST) -m comment --comment "$remarks" -j TPROXY --on-port $udp_redir_port --tproxy-mark 0x1/0x1
					$ipt_m -A PSW_ACL $(factor $ip "-s") $(factor $mac "-m mac --mac-source") -p udp $(factor $udp_redir_ports "-m multiport --dport") -m comment --comment "$remarks" -$(get_jump_mode $proxy_mode) $(get_action_chain $proxy_mode)$udp_node
				}
			fi
			[ -z "$ip" ] && {
				lower_mac=$(echo $mac | tr '[A-Z]' '[a-z]')
				ip=$(ip neigh show | grep -E "([0-9]{1,3}[\.]){3}[0-9]{1,3}" | grep $lower_mac | awk '{print $1}')
				[ -z "$ip" ] && {
					dhcp_index=$(uci show dhcp | grep $lower_mac | awk -F'.' '{print $2}')
					ip=$(uci -q get dhcp.$dhcp_index.ip)
				}
				[ -z "$ip" ] && ip=$(cat /tmp/dhcp.leases | grep -E "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | grep $lower_mac | awk '{print $3}')
			}
		fi
	}
}

filter_vpsip() {
	local server_ip use_ipv6 network_type
	use_ipv6=$(config_get $1 use_ipv6)
	network_type="ipv4"
	[ "$use_ipv6" == "1" ] && network_type="ipv6"
	server_ip=$(get_node_host_ip $1)
	[ -n "$server_ip" ] && {
		[ "$network_type" == "ipv4" ] && ipset -! add $IPSET_VPSIPLIST $server_ip >/dev/null 2>&1 &
	}
}

dns_hijack() {
	dnshijack=$(config_t_get global dns_53)
	if [ "$dnshijack" = "1" -o "$1" = "force" ]; then
		echolog "添加DNS劫持规则..."
		$ipt_n -I PSW -p udp --dport 53 -j REDIRECT --to-ports 53
		$ipt_n -I PSW -p tcp --dport 53 -j REDIRECT --to-ports 53
	fi
}

add_firewall_rule() {
	echolog "开始加载防火墙规则..."
	echolog "默认代理模式：$(get_action_chain_name $PROXY_MODE)"
	ipset -! create $IPSET_LANIPLIST nethash
	ipset -! create $IPSET_VPSIPLIST nethash
	ipset -! create $IPSET_ROUTER nethash
	ipset -! create $IPSET_GFW nethash
	ipset -! create $IPSET_CHN nethash
	ipset -! create $IPSET_BLACKLIST nethash && ipset flush $IPSET_BLACKLIST
	ipset -! create $IPSET_WHITELIST nethash && ipset flush $IPSET_WHITELIST

	sed -e "s/^/add $IPSET_CHN &/g" $RULE_PATH/chnroute | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	sed -e "s/^/add $IPSET_BLACKLIST &/g" $RULE_PATH/blacklist_ip | awk '{print $0} END{print "COMMIT"}' | ipset -! -R
	sed -e "s/^/add $IPSET_WHITELIST &/g" $RULE_PATH/whitelist_ip | awk '{print $0} END{print "COMMIT"}' | ipset -! -R

	ipset -! -R <<-EOF || return 1
		$(gen_laniplist | sed -e "s/^/add $IPSET_LANIPLIST /")
	EOF

	ISP_DNS=$(cat /tmp/resolv.conf.auto 2>/dev/null | grep -E -o "[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+" | sort -u | grep -v 0.0.0.0 | grep -v 127.0.0.1)
	[ -n "$ISP_DNS" ] && {
		for ispip in $ISP_DNS; do
			ipset -! add $IPSET_LANIPLIST $ispip >/dev/null 2>&1 &
		done
	}

	# 忽略特殊IP段
	lan_ip=$(ifconfig br-lan | grep "inet addr" | awk '{print $2}' | awk -F : '{print $2}') #路由器lan IP
	lan_ipv4=$(ip address show br-lan | grep -w "inet" | awk '{print $2}')                  #当前LAN IPv4段
	[ -n "$lan_ipv4" ] && ipset -! add $IPSET_LANIPLIST $lan_ipv4 >/dev/null 2>&1 &
	
	$ipt_n -N PSW
	$ipt_n -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_n -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_n -N PSW_ACL
	$ipt_n -N PSW_OUTPUT

	$ipt_m -N PSW
	$ipt_m -A PSW $(dst $IPSET_LANIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_VPSIPLIST) -j RETURN
	$ipt_m -A PSW $(dst $IPSET_WHITELIST) -j RETURN
	$ipt_m -N PSW_ACL
	$ipt_m -N PSW_OUTPUT

	if [[ "$TCP_NODE_NUM" -ge 1 ]] || [[ "$UDP_NODE_NUM" -ge 1 ]]; then
		local max_num=1
		[ "$TCP_NODE_NUM" -ge "$UDP_NODE_NUM" ] && max_num=$TCP_NODE_NUM
		if [ "$max_num" -ge 1 ]; then
			for i in $(seq 1 $max_num); do
				$ipt_n -N PSW_GLO$i
				$ipt_n -N PSW_GFW$i
				$ipt_n -N PSW_CHN$i
				$ipt_n -N PSW_HOME$i
				$ipt_n -N PSW_GAME$i
			
				$ipt_m -N PSW_GLO$i
				$ipt_m -N PSW_GFW$i
				$ipt_m -N PSW_CHN$i
				$ipt_m -N PSW_HOME$i
				$ipt_m -N PSW_GAME$i

				ip rule add fwmark 1 lookup 100
				ip route add local 0.0.0.0/0 dev lo table 100
			done
		fi
	fi

	if [ "$SOCKS5_NODE_NUM" -ge 1 ]; then
		for i in $(seq 1 $SOCKS5_NODE_NUM); do
			local k=$i
			eval node=\$SOCKS5_NODE$k
			if [ "$node" != "nil" ]; then
				local SOCKS5_NODE_PORT=$(config_get $node port)
				local SOCKS5_NODE_IP=$(get_node_host_ip $node)
				[ -n "$SOCKS5_NODE_IP" -a -n "$SOCKS5_NODE_PORT" ] && $ipt_n -A PSW -p tcp -d $SOCKS5_NODE_IP -m multiport --dports $SOCKS5_NODE_PORT -j RETURN
			fi
		done
	fi

	if [ "$TCP_NODE_NUM" -ge 1 ]; then
		for i in $(seq 1 $TCP_NODE_NUM); do
			local k=$i
			local ttl=14$k
			eval node=\$TCP_NODE$k
			eval local_port=\$TCP_REDIR_PORT$k
			# 生成TCP转发规则
			if [ "$node" != "nil" ]; then
				local TCP_NODE_PORT=$(config_get $node port)
				local TCP_NODE_IP=$(get_node_host_ip $node)
				local TCP_NODE_TYPE=$(echo $(config_get $node type) | tr 'A-Z' 'a-z')
				[ -n "$TCP_NODE_IP" -a -n "$TCP_NODE_PORT" ] && $ipt_n -A PSW -p tcp -d $TCP_NODE_IP -m multiport --dports $TCP_NODE_PORT -j RETURN
				if [ "$TCP_NODE_TYPE" == "brook" ]; then
					$ipt_m -A PSW_ACL -p tcp -m socket -j MARK --set-mark 1

					# $ipt_m -A PSW$k -p tcp $(dst $IPSET_BLACKLIST) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port
					# 全局模式
					$ipt_m -A PSW_GLO$k -p tcp -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

					# GFWLIST模式
					$ipt_m -A PSW_GFW$k -p tcp $(dst $IPSET_GFW) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

					# 大陆白名单模式
					$ipt_m -A PSW_CHN$k -p tcp $(dst $IPSET_CHN) -j RETURN
					$ipt_m -A PSW_CHN$k -p tcp -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

					# 回国模式
					$ipt_m -A PSW_HOME$k -p tcp $(dst $IPSET_CHN) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

					# 游戏模式
					$ipt_m -A PSW_GAME$k -p tcp $(dst $IPSET_CHN) -j RETURN

					# 用于本机流量转发
					[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
					$ipt_m -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS $(dst $IPSET_ROUTER) -j MARK --set-mark 1
					$ipt_m -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS $(dst $IPSET_GFW) -j MARK --set-mark 1
				else
					# 全局模式
					$ipt_n -A PSW_GLO$k -p tcp -j REDIRECT --to-ports $local_port

					# GFWLIST模式
					$ipt_n -A PSW_GFW$k -p tcp $(dst $IPSET_GFW) -j REDIRECT --to-ports $local_port

					# 大陆白名单模式
					$ipt_n -A PSW_CHN$k -p tcp $(dst $IPSET_CHN) -j RETURN
					#$ipt_n -A PSW_CHN$k -p tcp -m geoip ! --destination-country CN -j REDIRECT --to-ports $local_port
					$ipt_n -A PSW_CHN$k -p tcp -j REDIRECT --to-ports $local_port

					# 回国模式
					#$ipt_n -A PSW_HOME$k -p tcp -m geoip --destination-country CN -j REDIRECT --to-ports $local_port
					$ipt_n -A PSW_HOME$k -p tcp $(dst $IPSET_CHN) -j REDIRECT --to-ports $local_port

					# 游戏模式
					$ipt_n -A PSW_GAME$k -p tcp $(dst $IPSET_CHN) -j RETURN

					[ "$k" == 1 ] && {
						[ "$use_tcp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
							for dns in $DNS_FORWARD
							do
								local dns_ip=$(echo $dns | awk -F "#" '{print $1}')
								local dns_port=$(echo $dns | awk -F "#" '{print $2}')
								[ -z "$dns_port" ] && dns_port=53
								$ipt_n -I PSW 2 -p tcp -d $dns_ip --dport $dns_port -j REDIRECT --to-ports $local_port
							done
						}
						
						PRE_INDEX=1
						KP_INDEX=$($ipt_n -L PREROUTING --line-numbers | grep "KOOLPROXY" | sed -n '$p' | awk '{print $1}')
						ADBYBY_INDEX=$($ipt_n -L PREROUTING --line-numbers | grep "ADBYBY" | sed -n '$p' | awk '{print $1}')
						if [ -n "$KP_INDEX" -a -z "$ADBYBY_INDEX" ]; then
							PRE_INDEX=$(expr $KP_INDEX + 1)
						elif [ -z "$KP_INDEX" -a -n "$ADBYBY_INDEX" ]; then
							PRE_INDEX=$(expr $ADBYBY_INDEX + 1)
						elif [ -z "$KP_INDEX" -a -z "$ADBYBY_INDEX" ]; then
							PR_INDEX=$($ipt_n -L PREROUTING --line-numbers | grep "prerouting_rule" | sed -n '$p' | awk '{print $1}')
							[ -n "$PR_INDEX" ] && {
								PRE_INDEX=$(expr $PR_INDEX + 1)
							}
						fi
						$ipt_n -I PREROUTING $PRE_INDEX -j PSW
						
						# 用于本机流量转发
						$ipt_n -A OUTPUT -j PSW_OUTPUT
						$ipt_n -A PSW_OUTPUT $(dst $IPSET_LANIPLIST) -j RETURN
						[ "$use_tcp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
							for dns in $DNS_FORWARD
							do
								local dns_ip=$(echo $dns | awk -F "#" '{print $1}')
								local dns_port=$(echo $dns | awk -F "#" '{print $2}')
								[ -z "$dns_port" ] && dns_port=53
								$ipt_n -A PSW_OUTPUT -p tcp -d $dns_ip --dport $dns_port -j REDIRECT --to-ports $TCP_REDIR_PORT1
							done
						}
						$ipt_n -A PSW_OUTPUT $(dst $IPSET_VPSIPLIST) -j RETURN
						$ipt_n -A PSW_OUTPUT $(dst $IPSET_WHITELIST) -j RETURN
						
						[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -j RETURN
						
						$ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS $(dst $IPSET_BLACKLIST) -j REDIRECT --to-ports $TCP_REDIR_PORT1
						$ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS $(dst $IPSET_ROUTER) -j REDIRECT --to-ports $TCP_REDIR_PORT1
						[ "$LOCALHOST_PROXY_MODE" == "global" ] && $ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS -j REDIRECT --to-ports $TCP_REDIR_PORT1
						[ "$LOCALHOST_PROXY_MODE" == "gfwlist" ] && $ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS $(dst $IPSET_GFW) -j REDIRECT --to-ports $TCP_REDIR_PORT1
						[ "$LOCALHOST_PROXY_MODE" == "chnroute" ] && $ipt_n -A PSW_OUTPUT -p tcp -m multiport --dport $TCP_REDIR_PORTS -m set ! --match-set $IPSET_CHN dst -j REDIRECT --to-ports $TCP_REDIR_PORT1
					}
					# 重定所有流量到透明代理端口
					# $ipt_n -A PSW -p tcp -m ttl --ttl-eq $ttl -j REDIRECT --to $local_port
					echolog "IPv4 防火墙TCP转发规则加载完成！"
				fi
				if [ "$PROXY_IPV6" == "1" ]; then
					lan_ipv6=$(ip address show br-lan | grep -w "inet6" | awk '{print $2}') #当前LAN IPv6段
					$ip6t_n -N PSW
					$ip6t_n -N PSW_ACL
					$ip6t_n -A PREROUTING -j PSW
					[ -n "$lan_ipv6" ] && {
						for ip in $lan_ipv6; do
							$ip6t_n -A PSW -d $ip -j RETURN
						done
					}
					[ "$use_ipv6" == "1" -a -n "$server_ip" ] && $ip6t_n -A PSW -d $server_ip -j RETURN
					$ip6t_n -N PSW_GLO$k
					$ip6t_n -N PSW_GFW$k
					$ip6t_n -N PSW_CHN$k
					$ip6t_n -N PSW_HOME$k
					$ip6t_n -A PSW_GLO$k -p tcp -j REDIRECT --to $TCP_REDIR_PORT
					$ip6t_n -A PSW -j PSW_GLO$k
					#$ip6t_n -I OUTPUT -p tcp -j PSW
					echolog "IPv6防火墙规则加载完成！"
				fi
			fi
		done
		$ipt_n -A PSW -j PSW_ACL
	else
		echolog "主节点未选择，无法转发TCP！"
	fi

	if [ "$UDP_NODE_NUM" -ge 1 ]; then
		for i in $(seq 1 $UDP_NODE_NUM); do
			local k=$i
			eval node=\$UDP_NODE$k
			eval local_port=\$UDP_REDIR_PORT$k
			#  生成UDP转发规则
			if [ "$node" != "nil" ]; then
				local UDP_NODE_PORT=$(config_get $node port)
				local UDP_NODE_IP=$(get_node_host_ip $node)
				local UDP_NODE_TYPE=$(echo $(config_get $node type) | tr 'A-Z' 'a-z')
				[ -n "$UDP_NODE_IP" -a -n "$UDP_NODE_PORT" ] && $ipt_m -A PSW -p udp -d $UDP_NODE_IP -m multiport --dports $UDP_NODE_PORT -j RETURN
				[ "$UDP_NODE_TYPE" == "brook" ] && $ipt_m -A PSW_ACL -p udp -m socket -j MARK --set-mark 1
				#  全局模式
				$ipt_m -A PSW_GLO$k -p udp -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

				#  GFWLIST模式
				$ipt_m -A PSW_GFW$k -p udp $(dst $IPSET_ROUTER) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port
				$ipt_m -A PSW_GFW$k -p udp $(dst $IPSET_GFW) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

				#  大陆白名单模式
				$ipt_m -A PSW_CHN$k -p udp $(dst $IPSET_CHN) -j RETURN
				$ipt_m -A PSW_CHN$k -p udp -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

				#  回国模式
				$ipt_m -A PSW_HOME$k -p udp $(dst $IPSET_CHN) -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port

				#  游戏模式
				$ipt_m -A PSW_GAME$k -p udp $(dst $IPSET_CHN) -j RETURN
				$ipt_m -A PSW_GAME$k -p udp -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port
				
				[ "$k" == 1 ] && {
					# 用于本机流量转发
					$ipt_m -A OUTPUT -j PSW_OUTPUT
					$ipt_m -A PSW_OUTPUT -p udp $(dst $IPSET_LANIPLIST) -j RETURN
					[ "$use_udp_node_resolve_dns" == 1 -a -n "$DNS_FORWARD" ] && {
						for dns in $DNS_FORWARD
						do
							local dns_ip=$(echo $dns | awk -F "#" '{print $1}')
							local dns_port=$(echo $dns | awk -F "#" '{print $2}')
							[ -z "$dns_port" ] && dns_port=53
							$ipt_m -A PSW_OUTPUT -p udp -d $dns_ip --dport $dns_port -j MARK --set-mark 1
							$ipt_m -I PSW 2 -p udp -d $dns_ip --dport $dns_port -j TPROXY --tproxy-mark 0x1/0x1 --on-port $local_port
						done
					}
					$ipt_m -A PSW_OUTPUT -p udp $(dst $IPSET_VPSIPLIST) -j RETURN
					$ipt_m -A PSW_OUTPUT -p udp $(dst $IPSET_WHITELIST) -j RETURN
					
					[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -j RETURN
					
					$ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_REDIR_PORTS $(dst $IPSET_BLACKLIST) -j MARK --set-mark 1
					$ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_REDIR_PORTS $(dst $IPSET_ROUTER) -j MARK --set-mark 1
					[ "$LOCALHOST_PROXY_MODE" == "global" ] && $ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_REDIR_PORTS -j MARK --set-mark 1
					[ "$LOCALHOST_PROXY_MODE" == "gfwlist" ] && $ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_REDIR_PORTS $(dst $IPSET_GFW) -j MARK --set-mark 1
					[ "$LOCALHOST_PROXY_MODE" == "chnroute" ] && $ipt_m -A PSW_OUTPUT -p udp -m multiport --dport $UDP_REDIR_PORTS -m set ! --match-set $IPSET_CHN dst -j MARK --set-mark 1
				}

				echolog "IPv4 防火墙UDP转发规则加载完成！"
			fi
		done
		$ipt_m -A PSW -j PSW_ACL
	else
		echolog "UDP节点未选择，无法转发UDP！"
	fi
		
	if [ -n "$balancing_node_address" ]; then
		balancing_node_address=$(echo -e $balancing_node_address)
		for balancing_node in $balancing_node_address
		do
			local ip=$(echo $balancing_node | awk -F ":" '{print $1}')
			local port=$(echo $balancing_node | awk -F ":" '{print $2}')
			$ipt_n -I PSW 2 -p tcp -d $ip --dport $port -j RETURN
			$ipt_n -I PSW_OUTPUT 2 -p tcp -d $ip --dport $port -j RETURN
			$ipt_m -I PSW 2 -p udp -d $ip --dport $port -j RETURN
			$ipt_m -I PSW_OUTPUT 2 -p udp -d $ip --dport $port -j RETURN
		done
	fi

	$ipt_m -A PREROUTING -j PSW
	
	#  加载ACLS
	config_foreach load_acl "acl_rule"

	#  加载默认代理模式
	if [ "$PROXY_MODE" == "disable" ]; then
		[ "$TCP_NODE1" != "nil" ] && $ipt_n -A PSW_ACL -p tcp -m comment --comment "Default" -j $(get_action_chain $PROXY_MODE)
		[ "$UDP_NODE1" != "nil" ] && $ipt_m -A PSW_ACL -p udp -m comment --comment "Default" -j $(get_action_chain $PROXY_MODE)
	else
		[ "$TCP_NODE1" != "nil" ] && {
			[ "$TCP_NO_REDIR_PORTS" != "disable" ] && $ipt_n -A PSW_ACL -p tcp -m multiport --dport $TCP_NO_REDIR_PORTS -m comment --comment "Default" -j RETURN
			$ipt_n -A PSW_ACL -p tcp $(dst $IPSET_BLACKLIST) -m comment --comment "Default" -j REDIRECT --to-ports $TCP_REDIR_PORT1
			$ipt_n -A PSW_ACL -p tcp -m multiport --dport $TCP_REDIR_PORTS -m comment --comment "Default" -j $(get_action_chain $PROXY_MODE)1
		}
		[ "$UDP_NODE1" != "nil" ] && {
			[ "$UDP_NO_REDIR_PORTS" != "disable" ] && $ipt_m -A PSW_ACL -p udp -m multiport --dport $UDP_NO_REDIR_PORTS -m comment --comment "Default" -j RETURN
			$ipt_m -A PSW_ACL -p udp $(dst $IPSET_BLACKLIST) -m comment --comment "Default" -j TPROXY --on-port $UDP_REDIR_PORT1 --tproxy-mark 0x1/0x1
			$ipt_m -A PSW_ACL -p udp -m multiport --dport $UDP_REDIR_PORTS -m comment --comment "Default" -j $(get_action_chain $PROXY_MODE)1
		}
	fi
	
	#  过滤所有节点IP，暂时关闭，节点一多会解析很久导致启动超慢。。。
	# config_foreach filter_vpsip "nodes"
}

del_firewall_rule() {
	echolog "删除所有防火墙规则..."
	ipv6_output_ss_exist=$($ip6t_n -L OUTPUT 2>/dev/null | grep -c "PSW")
	[ -n "$ipv6_output_ss_exist" ] && {
		until [ "$ipv6_output_ss_exist" = 0 ]; do
			rules=$($ip6t_n -L OUTPUT --line-numbers | grep "PSW" | awk '{print $1}')
			for rule in $rules; do
				$ip6t_n -D OUTPUT $rule 2>/dev/null
				break
			done
			ipv6_output_ss_exist=$(expr $ipv6_output_ss_exist - 1)
		done
	}

	$ipt_n -D PREROUTING -j PSW 2>/dev/null
	$ipt_n -D OUTPUT -j PSW_OUTPUT 2>/dev/null
	$ipt_n -F PSW 2>/dev/null && $ipt_n -X PSW 2>/dev/null
	$ipt_n -F PSW_ACL 2>/dev/null && $ipt_n -X PSW_ACL 2>/dev/null
	$ipt_n -F PSW_OUTPUT 2>/dev/null && $ipt_n -X PSW_OUTPUT 2>/dev/null
	
	$ipt_m -D PREROUTING -j PSW 2>/dev/null
	$ipt_m -D OUTPUT -j PSW_OUTPUT 2>/dev/null
	$ipt_m -F PSW 2>/dev/null && $ipt_m -X PSW 2>/dev/null
	$ipt_m -F PSW_ACL 2>/dev/null && $ipt_m -X PSW_ACL 2>/dev/null
	$ipt_m -F PSW_OUTPUT 2>/dev/null && $ipt_m -X PSW_OUTPUT 2>/dev/null

	$ip6t_n -D PREROUTING -j PSW 2>/dev/null
	$ip6t_n -D OUTPUT -j PSW_OUTPUT 2>/dev/null
	$ip6t_n -F PSW 2>/dev/null && $ip6t_n -X PSW 2>/dev/null
	$ip6t_n -F PSW_ACL 2>/dev/null && $ip6t_n -X PSW_ACL 2>/dev/null
	$ip6t_n -F PSW_OUTPUT 2>/dev/null && $ip6t_n -X PSW_OUTPUT 2>/dev/null

	local max_num=5
	if [ "$max_num" -ge 1 ]; then
		for i in $(seq 1 $max_num); do
			local k=$i
			$ipt_n -F PSW_GLO$k 2>/dev/null && $ipt_n -X PSW_GLO$k 2>/dev/null
			$ipt_n -F PSW_GFW$k 2>/dev/null && $ipt_n -X PSW_GFW$k 2>/dev/null
			$ipt_n -F PSW_CHN$k 2>/dev/null && $ipt_n -X PSW_CHN$k 2>/dev/null
			$ipt_n -F PSW_GAME$k 2>/dev/null && $ipt_n -X PSW_GAME$k 2>/dev/null
			$ipt_n -F PSW_HOME$k 2>/dev/null && $ipt_n -X PSW_HOME$k 2>/dev/null
			
			$ipt_m -F PSW_GLO$k 2>/dev/null && $ipt_m -X PSW_GLO$k 2>/dev/null
			$ipt_m -F PSW_GFW$k 2>/dev/null && $ipt_m -X PSW_GFW$k 2>/dev/null
			$ipt_m -F PSW_CHN$k 2>/dev/null && $ipt_m -X PSW_CHN$k 2>/dev/null
			$ipt_m -F PSW_GAME$k 2>/dev/null && $ipt_m -X PSW_GAME$k 2>/dev/null
			$ipt_m -F PSW_HOME$k 2>/dev/null && $ipt_m -X PSW_HOME$k 2>/dev/null

			$ip6t_n -F PSW_GLO$k 2>/dev/null && $ip6t_n -X PSW_GLO$k 2>/dev/null
			$ip6t_n -F PSW_GFW$k 2>/dev/null && $ip6t_n -X PSW_GFW$k 2>/dev/null
			$ip6t_n -F PSW_CHN$k 2>/dev/null && $ip6t_n -X PSW_CHN$k 2>/dev/null
			$ip6t_n -F PSW_HOME$k 2>/dev/null && $ip6t_n -X PSW_HOME$k 2>/dev/null

			ip_rule_exist=$(ip rule show | grep "from all fwmark 0x1 lookup 100" | grep -c 100)
			if [ ! -z "$ip_rule_exist" ]; then
				until [ "$ip_rule_exist" = 0 ]; do
					ip rule del fwmark 1 lookup 100
					ip_rule_exist=$(expr $ip_rule_exist - 1)
				done
			fi
			ip route del local 0.0.0.0/0 dev lo table 100 2>/dev/null
		done
	fi

	ipset -F $IPSET_LANIPLIST >/dev/null 2>&1 && ipset -X $IPSET_LANIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_VPSIPLIST >/dev/null 2>&1 && ipset -X $IPSET_VPSIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_ROUTER >/dev/null 2>&1 && ipset -X $IPSET_ROUTER >/dev/null 2>&1 &
	#ipset -F $IPSET_GFW >/dev/null 2>&1 && ipset -X $IPSET_GFW >/dev/null 2>&1 &
	#ipset -F $IPSET_CHN >/dev/null 2>&1 && ipset -X $IPSET_CHN >/dev/null 2>&1 &
	ipset -F $IPSET_BLACKLIST >/dev/null 2>&1 && ipset -X $IPSET_BLACKLIST >/dev/null 2>&1 &
	ipset -F $IPSET_WHITELIST >/dev/null 2>&1 && ipset -X $IPSET_WHITELIST >/dev/null 2>&1 &
}

flush_ipset() {
	ipset -F $IPSET_LANIPLIST >/dev/null 2>&1 && ipset -X $IPSET_LANIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_VPSIPLIST >/dev/null 2>&1 && ipset -X $IPSET_VPSIPLIST >/dev/null 2>&1 &
	ipset -F $IPSET_ROUTER >/dev/null 2>&1 && ipset -X $IPSET_ROUTER >/dev/null 2>&1 &
	ipset -F $IPSET_GFW >/dev/null 2>&1 && ipset -X $IPSET_GFW >/dev/null 2>&1 &
	ipset -F $IPSET_CHN >/dev/null 2>&1 && ipset -X $IPSET_CHN >/dev/null 2>&1 &
	ipset -F $IPSET_BLACKLIST >/dev/null 2>&1 && ipset -X $IPSET_BLACKLIST >/dev/null 2>&1 &
	ipset -F $IPSET_WHITELIST >/dev/null 2>&1 && ipset -X $IPSET_WHITELIST >/dev/null 2>&1 &
}

start() {
	add_firewall_rule
	dns_hijack
}

stop() {
	del_firewall_rule
}

case $1 in
flush_ipset)
	flush_ipset
	;;
stop)
	stop
	;;
start)
	start
	;;
*) ;;
esac
