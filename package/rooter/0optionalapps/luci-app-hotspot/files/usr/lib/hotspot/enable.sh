#!/bin/sh
. /lib/functions.sh

SET=$1

uci set travelmate.global.trm_enabled=$SET
uci commit travelmate

if [ $SET = "1" ]; then
	AU=$(uci get travelmate.global.trm_auto)
	hkillall travelmate.sh
	if [ $AU = "1" ]; then
		uci set travelmate.global.ssid="8"
		uci commit travelmate
		uci -q set wireless.wwan.encryption="none"
		uci -q set wireless.wwan.key=
		uci set wireless.wwan.ssid="Hotspot Manager Interface"
       	uci -q commit wireless
		/usr/lib/hotspot/travelmate.sh &
	fi
else
	hkillall travelmate.sh
	rm -f /tmp/hotman
	uci set travelmate.global.ssid="7"
	uci set travelmate.global.trm_enabled="0"
	uci commit travelmate
	uci -q set wireless.wwan.disabled=1
	uci set wireless.wwan.ssid="Hotspot Manager Interface"
	uci -q commit wireless
	ubus call network.interface.wwan down
    ubus call network reload
	#ifdown wwan
	#/etc/init.d/network reload
fi