#!/bin/sh

# 检查 network.globals.ula_prefix 是否存在且不为空
ula_prefix=$(uci get network.globals.ula_prefix 2>/dev/null)

if [ -n "$ula_prefix" ]; then
	uci set dhcp.wan6=dhcp
	uci set dhcp.wan6.interface='wan6'
	uci set dhcp.wan6.ignore='1'

	uci set dhcp.lan.force='1'
	uci set dhcp.lan.ra='hybrid'
	uci set dhcp.lan.ra_default='1'
	uci set dhcp.lan.max_preferred_lifetime='1800'
	uci set dhcp.lan.max_valid_lifetime='3600'

	uci del dhcp.lan.dhcpv6
	uci del dhcp.lan.ra_flags
	uci del dhcp.lan.ra_slaac
	uci add_list dhcp.lan.ra_flags='none'

	uci commit dhcp

	uci set network.wan6.reqaddress='try'
	uci set network.wan6.reqprefix='auto'
	uci set network.lan.ip6assign='64'
	uci set network.lan.ip6ifaceid='eui64'
	uci set network.globals.packet_steering='0'
	uci del network.globals.ula_prefix

	uci commit network
fi

exit 0
