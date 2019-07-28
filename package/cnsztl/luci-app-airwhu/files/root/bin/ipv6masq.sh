#!/bin/sh /etc/rc.common
#Author:KyleRicardo
#Email:shaoyz714@126.com

action=$1

if [ "$action"x = "install"x ]; then 
	uci delete network.globals.ula_prefix

	uci set network.wan6.peerdns='0'
	uci set network.wan6.dns='2001:da8:202:10::36'

	uci delete network.lan.ip6assign
	uci set network.lan.ip6addr='BABE:BABE:BABE:BABE::1/64'
	uci set network.lan.ip6prefix='BABE:BABE:BABE:BABE::/64'

	uci set dhcp.lan.ra_management='1'
	uci set dhcp.lan.ra_default='1'

	uci set $(uci show firewall|grep Allow-ICMPv6-Forward|sed -e 's/.name[^ ]*'//).enabled='0'

	uci commit


elif [ "$action"x = "uninstall"x ]; then 
	uci set network.globals.ula_prefix='fdad:91b7:54bb::/48'

	uci delete network.wan6.peerdns='0'
	uci delete network.wan6.dns='2001:da8:202:10::36'

	uci set network.lan.ip6assign='60'
	uci delete network.lan.ip6addr='BABE:BABE:BABE:BABE::1/64'
	uci delete network.lan.ip6prefix='BABE:BABE:BABE:BABE::/64'

	uci delete dhcp.lan.ra_management='1'
	uci delete dhcp.lan.ra_default='1'

	uci delete $(uci show firewall|grep Allow-ICMPv6-Forward|sed -e 's/.name[^ ]*'//).enabled

	uci commit
fi
