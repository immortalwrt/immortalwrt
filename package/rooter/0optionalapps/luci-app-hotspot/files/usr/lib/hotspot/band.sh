#!/bin/sh 

log() {
	logger -t "band change" "$@"
}

BAND=$1
let BAND=BAND-1

WW="radio"$BAND

wifi up
uci set wireless.wwan.device=$WW
uci set wireless.wwan.encryption="none"
uci set wireless.wwan.disabled="1"
uci commit wireless
uci set travelmate.global.ssid="6"
uci commit travelmate
wifi up
result=`ps | grep -i "travelmate.sh" | grep -v "grep" | wc -l`
if [ $result -ge 1 ]
then
	logger -t TRAVELMATE-DEBUG "Travelmate already running"
else
	/usr/lib/hotspot/travelmate.sh &
fi
sleep 10
uci set travelmate.global.ssid="Wifi Radio finished changing"
uci commit travelmate
exit 0