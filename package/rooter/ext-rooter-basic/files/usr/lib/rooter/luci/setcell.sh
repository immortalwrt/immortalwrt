#!/bin/sh
 
ROOTER=/usr/lib/rooter

log() {
	logger -t "Lock Cell" "$@"
}

dat="$1"

dat1=$(echo $dat | tr "|" ",")
dat2=$(echo $dat1 | cut -d, -f1)
if [ $dat2 = "0" ]; then
	uci set custom.bandlock.cenable='0'
else
	ear=$(echo $dat1 | cut -d, -f2)
	pc=$(echo $dat1 | cut -d, -f3)
	ear1=$(echo $dat1 | cut -d, -f4)
	pc1=$(echo $dat1 | cut -d, -f5)
	ear2=$(echo $dat1 | cut -d, -f6)
	pc2=$(echo $dat1 | cut -d, -f7)
	ear3=$(echo $dat1 | cut -d, -f8)
	pc3=$(echo $dat1 | cut -d, -f9)
	uci set custom.bandlock.cenable='1'
	uci set custom.bandlock.earfcn=$ear
	uci set custom.bandlock.pci=$pc
	uci set custom.bandlock.earfcn1=$ear1
	uci set custom.bandlock.pci1=$pc1
	uci set custom.bandlock.earfcn2=$ear2
	uci set custom.bandlock.pci2=$pc2
	uci set custom.bandlock.earfcn3=$ear3
	uci set custom.bandlock.pci3=$pc3
fi
uci commit custom

ifname1="ifname"
if [ -e /etc/newstyle ]; then
	ifname1="device"
fi


CURRMODEM=$(uci get modem.general.miscnum)
COMMPORT="/dev/ttyUSB"$(uci get modem.modem$CURRMODEM.commport)
CPORT=$(uci -q get modem.modem$CURRMODEM.commport)

ATCMDD="at+qnwlock=\"common/4g\""
OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
if `echo $OX | grep "ERROR" 1>/dev/null 2>&1`
then
	ATCMDD="at+qnwlock=\"common/lte\",0"
else
	ATCMDD=$ATCMDD",0"
fi
OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
log "$OX"
sleep 5		
ATCMDD="AT+CFUN=1,1"
OX=$($ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD")
log "Hard modem reset done on /dev/ttyUSB$CPORT to reload drivers"
ifdown wan$CURRMODEM
uci delete network.wan$CURRMODEM
uci set network.wan$CURRMODEM=interface
uci set network.wan$CURRMODEM.proto=dhcp
uci set network.wan$CURRMODEM.${ifname1}="wan"$CURRMODEM
uci set network.wan$CURRMODEM.metric=$CURRMODEM"0"
uci commit network
/etc/init.d/network reload
ifdown wan$CURRMODEM
echo "1" > /tmp/modgone
log "Setting Modem Removal flag (1)"
