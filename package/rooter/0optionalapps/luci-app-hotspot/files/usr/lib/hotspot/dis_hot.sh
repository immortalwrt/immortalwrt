#!/bin/sh
. /lib/functions.sh

rm -f /tmp/hotman
uci set travelmate.global.ssid="7"
uci commit travelmate
uci -q set wireless.wwan.disabled=1
uci set wireless.wwan.ssid="Hotspot Manager Interface"
uci -q commit wireless
#ifdown wwan
#ubus call network reload
/etc/init.d/network reload

