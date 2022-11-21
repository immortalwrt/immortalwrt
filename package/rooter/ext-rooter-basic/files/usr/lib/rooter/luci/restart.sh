#!/bin/sh

ROOTER=/usr/lib/rooter
ROOTER_LINK="/tmp/links"

log() {
	modlog "Modem Restart/Diisconnect $CURRMODEM" "$@"
}

ifname1="ifname"
if [ -e /etc/newstyle ]; then
	ifname1="device"
fi

CURRMODEM=$1
CPORT=$(uci -q get modem.modem$CURRMODEM.commport)
INTER=$(uci get modem.modeminfo$CURRMODEM.inter)

if [ ! -z "$2" ]; then # disconnect
	uci set modem.modem$CURRMODEM.connected=0
	uci commit modem
	jkillall getsignal$CURRMODEM
	rm -f $ROOTER_LINK/getsignal$CURRMODEM
	jkillall con_monitor$CURRMODEM
	rm -f $ROOTER_LINK/con_monitor$CURRMODEM
	ifdown wan$INTER
	$ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "reset.gcom" "$CURRMODEM"
else # restart
	uVid=$(uci get modem.modem$CURRMODEM.uVid)
	uPid=$(uci get modem.modem$CURRMODEM.uPid)
	#if [ $uVid != "2c7c" ]; then
		if [ ! -z "$CPORT" ]; then
			ATCMDD="AT+CFUN=1,1"
			OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
		fi
		log "Hard modem reset done"
	#fi
	ifdown wan$INTER
	uci delete network.wan$CURRMODEM
	uci set network.wan$CURRMODEM=interface
	uci set network.wan$CURRMODEM.proto=dhcp
	uci set network.wan$CURRMODEM.${ifname1}="wan"$CURRMODEM
	uci set network.wan$CURRMODEM.metric=$CURRMODEM"0"
	uci commit network
	/etc/init.d/network reload
	echo "1" > /tmp/modgone
	log "Hard USB reset done"

	PORT="usb$CURRMODEM"
	echo $PORT > /sys/bus/usb/drivers/usb/unbind
	sleep 35
	echo $PORT > /sys/bus/usb/drivers/usb/bind
fi
