#!/bin/sh

ROOTER=/usr/lib/rooter

log() {
	logger -t "SimLock " "$@"
}

imsi() {
	rm -f /tmp/msimdatax$CURRMODEM
	IMSI=$(uci -q get modem.modem$CURRMODEM.imsi)
	if [ "$IMSI" = "Unknown" ]; then
		while IFS= read -r line; do
		log "Read SIM Data"
			ln=$line
			echo "$ln" >> /tmp/msimdatax$CURRMODEM
			read -r line
			ln=$line
			echo "$ln" >> /tmp/msimdatax$CURRMODEM
			read -r line
			ln="<div style=\"color :red; \"><strong>Wrong Pin Used</strong></div>"
			echo "$ln" >> /tmp/msimdatax$CURRMODEM
			read -r line
			ln=$line
			echo "$ln" >> /tmp/msimdatax$CURRMODEM
			read -r line
			ln=$line
			echo "$ln" >> /tmp/msimdatax$CURRMODEM
			echo " " >> /tmp/msimdatax$CURRMODEM
			mv -f /tmp/msimdatax$CURRMODEM /tmp/msimdata$CURRMODEM
			echo "1" > /tmp/simpin$CURRMODEM
			break
		done < /tmp/msimdata$CURRMODEM
	fi
}

CURRMODEM=$1
CPORT=$(uci get modem.modem$CURRMODEM.commport)
rm -f /tmp/simpin$CURRMODEM

IMSI=$(uci -q get modem.modem$CURRMODEM.imsi)
ICCID=$(uci -q get modem.modem$CURRMODEM.iccid)
if [ "$IMSI" = "Unknown" -a "$ICCID" = "Unknown" ]; then
	echo "2" > /tmp/simpin$CURRMODEM
	exit 0
fi

spin=$(uci -q get modem.modeminfo$CURRMODEM.pincode)
if [ ! -z $spin ]; then
	export PINCODE="$spin"
	OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "setpin.gcom" "$CURRMODEM")
	log "PIN SET $OX"
	$ROOTER/common/gettype.sh $CURRMODEM
	imsi
else
	spin=$(uci -q get profile.simpin.pin)
	if [ ! -z $spin ]; then
		export PINCODE="$spin"
		OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "setpin.gcom" "$CURRMODEM")
		log "PIN SET $OX"
		$ROOTER/common/gettype.sh $CURRMODEM
		imsi
	fi
fi
