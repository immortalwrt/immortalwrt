#!/bin/sh

log() {
	logger -t "wrtbwmon" "$@"
}

enable=$1

WX=$(uci -q get custom.bwallocate.lock)
if [ "$WX" = "1" ]; then
	enable="1"
fi
uci set bwmon.general.enabled=$enable
uci commit bwmon
WW=$(uci -q get bwmon.general.enabled)
if [ "$WW" = "1"  ]; then
	result=`ps | grep -i "wrtbwmon.sh" | grep -v "grep" | wc -l`
	if [ $result -lt 1 ]; then
		/usr/lib/bwmon/wrtbwmon.sh &
		/usr/lib/bwmon/create.sh &
	fi
else
	PID=$(ps |grep "wrtbwmon.sh" | grep -v grep |head -n 1 | awk '{print $1}')
	PID1=$(ps |grep "create.sh" | grep -v grep |head -n 1 | awk '{print $1}')
	if [ ! -z "$PID" ]; then
		kill -9 $PID
		kill -9 $PID1
	fi
fi