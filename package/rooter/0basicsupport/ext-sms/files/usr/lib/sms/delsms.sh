#!/bin/sh

ROOTER=/usr/lib/rooter

log() {
	logger -t "Delete SMS" "$@"
}

CURRMODEM=$1
shift 1
SLOTS="$@"

COMMPORT="/dev/ttyUSB"$(uci get modem.modem$CURRMODEM.commport)

SMSLOC=$(uci -q get modem.modem$CURRMODEM.smsloc)

LOCKDIR="/tmp/smslock$CURRMODEM"
PIDFILE="${LOCKDIR}/PID"

while [ 1 -lt 6 ]; do
	if mkdir "${LOCKDIR}" &>/dev/null; then
		echo "$$" > "${PIDFILE}"
		for SLOT in $SLOTS
		do
			ATCMDD="AT+CPMS=\"$SMSLOC\";+CMGD=$SLOT"
			OX=$($ROOTER/gcom/gcom-locked "$COMMPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
		done
		uci set modem.modem$CURRMODEM.smsnum=999
		uci commit modem
		break
	else
		OTHERPID="$(cat "${PIDFILE}")"
		if [ $? = 0 ]; then
			if ! kill -0 $OTHERPID &>/dev/null; then
				rm -rf "${LOCKDIR}"
			fi
		fi
		sleep 1
	fi
done
rm -rf "${LOCKDIR}"
