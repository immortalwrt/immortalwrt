#!/bin/sh

LED=0
SM=$(uci get system.4G1)
if [ -z $SM ]; then
	uci set system.4G1=led
	uci set system.4G1.name="4G1"
	uci set system.4G1.sysfs="green:4g1"
	uci set system.4G1.trigger="netdev"
	uci set system.4G1.dev="wwan0"
	uci set system.4G1.mode="link tx rx"
	LED=1
fi
SM=$(uci get system.4G2)
if [ -z $SM ]; then
	uci set system.4G2=led
	uci set system.4G2.name="4G2"
	uci set system.4G2.sysfs="green:4g2"
	uci set system.4G2.trigger="netdev"
	uci set system.4G2.dev="wwan1"
	uci set system.4G2.mode="link tx rx"
	LED=1
fi

if [ $LED -eq 1 ]; then
	uci commit system
	/etc/init.d/led restart
fi
