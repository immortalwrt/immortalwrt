#!/bin/sh

ROOTER=/usr/lib/rooter
ROOTER_LINK="/tmp/links"

log() {
	logger -t "SIM Lock" "$@"
}

CURRMODEM=$1
CPORT=$(uci get modem.modem$CURRMODEM.commport)

ATCMDD="at+cpin?"
OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
RDY=$(echo "$OX" | grep "READY")
if [ -z "$RDY" ]; then
	ATCMDD="AT+CLCK=\"SC\",2"
	OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
	RDY=$(echo "$OX" | grep "1")
	if [ ! -z "$RDY" ]; then
		PIN=$(uci -q get modem.modeminfo$CURRMODEM.pincode)
		if [ -z "$PIN" ]; then
			spin=$(uci -q get profile.simpin.pin)
			if [ -z $spin ]; then
				echo "0" > /tmp/simpin$CURRMODEM
			fi
		fi
	fi
fi