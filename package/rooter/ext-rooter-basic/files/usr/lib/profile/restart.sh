#!/bin/sh
. /lib/functions.sh

ROOTER=/usr/lib/rooter

log() {
	logger -t "Restart" "$@"
}

sleep 3

CURRMODEM=1
CPORT=$(uci -q get modem.modem$CURRMODEM.commport)

if [ ! -z $CPORT ]; then
	ATCMDD="AT+CFUN=1,1"
	$ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD"
fi