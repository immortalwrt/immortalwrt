#!/bin/sh

uci -q get dhcp.@dnsmasq[0].mini_ttl >"/dev/null" && {
	uci rename dhcp.@dnsmasq[0].mini_ttl="min_cache_ttl"
	uci commit dhcp
}

[ "$(uci get dhcp.@dnsmasq[0].resolvfile)" = "/tmp/resolv.conf.auto" ] && {
	uci set dhcp.@dnsmasq[0].resolvfile="/tmp/resolv.conf.d/resolv.conf.auto"
	uci commit dhcp
}

exit 0
