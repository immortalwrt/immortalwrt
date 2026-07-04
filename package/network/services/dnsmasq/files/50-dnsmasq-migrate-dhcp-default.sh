#!/bin/sh
# dnsmasq.init used to treat an unset 'dhcpv4'/'dhcpv6' option as
# implicitly enabled. Since the fallback default was changed to
# 'disabled' (to match odhcpd), any existing 'config dhcp' section
# that never had to set these options explicitly silently stops
# serving that protocol. Pin the old behavior for configs that
# predate the change.

. /lib/functions.sh

migrate_dhcp() {
	local cfg="$1"
	local dhcpv4 dhcpv6

	config_get dhcpv4 "$cfg" dhcpv4
	config_get dhcpv6 "$cfg" dhcpv6

	[ -n "$dhcpv4" ] || uci -q set dhcp."$cfg".dhcpv4='server'
	[ -n "$dhcpv6" ] || uci -q set dhcp."$cfg".dhcpv6='server'
}

config_load dhcp
config_foreach migrate_dhcp dhcp

uci -q commit dhcp
exit 0
