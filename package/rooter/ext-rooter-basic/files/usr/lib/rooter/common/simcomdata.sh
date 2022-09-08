#!/bin/sh

ROOTER=/usr/lib/rooter

CURRMODEM=$1
COMMPORT=$2

decode_bw() {
	BW=$(echo $BW | grep -o "[0-5]\{1\}")
	case $BW in
		"0")
			BW="1.4" ;;
		"1")
			BW="3" ;;
		"2")
			BW="5" ;;
		"3")
			BW="10" ;;
		"4")
			BW="15" ;;
		"5")
			BW="20" ;;
	esac
}

OX=$($ROOTER/gcom/gcom-locked "$COMMPORT" "simcominfo.gcom" "$CURRMODEM")

OX=$(echo $OX | tr 'a-z' 'A-Z')

REGXa="+CPSI:[ ]*LTE,ONLINE,[0-9]\{3\}-[0-9]\{2,3\},0X[0-9A-F]\{1,5\},[0-9]\{3,9\},[0-9]\{1,3\},[-A-Z0-9]\+,[0-9]\{1,6\},[0-5],[0-5],-[0-9]\{1,3\},-[0-9]\{1,4\},[-0-9]\{1,5\},[0-9]\{1,2\}"

REGXb="+CPSI:[ ]*NR5G[_ANS]*,[0-9]\{1,3\},[0-9]\{6\},-[0-9]\{1,4\},-[0-9]\{1,4\},[-0-9]\{1,4\}"

REGXc="+CPSI:[ ]*NR5G_SA,ONLINE,[0-9]\{3\}-[0-9]\{2,3\},0X[0-9A-F]\{1,6\},[0-9]\{3,13\},[0-9]\{1,3\},[^,]\+,[0-9]\{6,7\},-[0-9]\{1,4\},-[0-9]\{1,4\},[-0-9]\{1,4\}"

REGXz="+CMGRMI: CA_SCELL,[0-9]\{2,6\},[^,]\+,[0-9]\{1,3\},[0-5],[^,]\+,[0-9]\{1,3\},[^,]\+,[^,]\+,[^,]\+,[^,]\+,[^,]\+,1"

CHANNEL="-"
ECIO="-"
RSCP="-"
ECIO1=" "
RSCP1=" "
MODE="-"
NETMODE="-"
LBAND="-"
PCI="-"
CTEMP="-"
MODE=""
SINR="-"

CSQ=$(echo "$OX" | grep -o "+CSQ: [0-9]\{1,2\}" | grep -o "[0-9]\{1,2\}")
if [ $CSQ = "99" ]; then
	CSQ=""
fi
if [ -n "$CSQ" ]; then
	CSQ_PER=$(($CSQ * 100/31))"%"
	CSQ_RSSI=$((2 * CSQ - 113))" dBm"
else
	CSQ="-"
	CSQ_PER="-"
	CSQ_RSSI="-"
fi

CPSIa=$(echo "$OX" | grep -o "$REGXa")
if [ -n "$CPSIa" ]; then
	CPSIb=$(echo "$OX" | grep -o "$REGXb")
	PCI=$(echo $CPSIa | cut -d, -f6)
	LBAND=$(echo $CPSIa | cut -d, -f7 | grep -o "[0-9]\{1,3\}")
	CHANNEL=$(echo $CPSIa | cut -d, -f8)
	BW=$(echo $CPSIa | cut -d, -f9)
	decode_bw
	BWD=$BW
	BW=$(echo $CPSIa | cut -d, -f10)
	decode_bw
	BWU=$BW
	LBAND="B"$LBAND" (Bandwidth $BWD MHz Down | $BWU MHz Up)"
	ECIO=$(($(echo $CPSIa | cut -d, -f11) / 10))
	RSCP=$(($(echo $CPSIa | cut -d, -f12) / 10))
	SINR=$((($(echo $CPSIa | cut -d, -f14) * 2) - 20))" dB"
	if [ -n "$CPSIb" ]; then
		MODE="LTE/NR EN-DC"
		PCI=$PCI", "$(echo $CPSIb | cut -d, -f2)
		NCHAN=$(echo $CPSIb | cut -d, -f3)
		CHANNEL="$CHANNEL, $NCHAN"
		NBAND=$("$ROOTER/chan2band.sh" "$NCHAN")
		if [ "$NBAND" = "-" ]; then
			LBAND=$LBAND"<br />nxx (unknown NR5G band)"
		else
			LBAND=$LBAND"<br />"$NBAND
		fi
		RSCP=$RSCP" dBm<br />"$(($(echo $CPSIb | cut -d, -f4) / 10))
		ECIO=$ECIO" dB<br />"$(($(echo $CPSIb | cut -d, -f5) / 10))
		SSINR=$(echo $CPSIb | cut -d, -f6)
		if [ $SSINR -lt 255 ]; then
			SINR=$SINR"<br />"$((($SSINR / 5) - 20))" dB"
		fi
	else
		MODE="LTE"
	fi
	CPSIc=""
else
	CPSIc=$(echo "$OX" | grep -o "$REGXc")
	if [ -n "$CPSIc" ]; then
		MODE="NR5G"
		PCI=$(echo $CPSIc | cut -d, -f6)
		LBAND="n"$(echo $CPSIc | cut -d, -f7 | grep -o "BAND[0-9]\{1,3\}" | grep -o "[0-9]\+")
		CHANNEL=$(echo $CPSIc | cut -d, -f8)
		RSCP=$(($(echo $CPSIc | cut -d, -f9) / 10))
		ECIO=$(($(echo $CPSIc | cut -d, -f10) / 10))
		if [ "$CSQ_PER" = "-" ]; then
			CSQ_PER=$((100 - (($RSCP + 31) * 100/-125)))"%"
		fi
		SINR=$(($(echo $CPSIc | cut -d, -f11) / 10))" dB"
	fi
	CPSIb=""
fi

CAINFO=$(echo "$OX" | grep -o "$REGXz" | tr ' ' ':')
if [ -n "$CAINFO" ]; then
	for CASV in $(echo "$CAINFO"); do
		LBAND=$LBAND"<br />B"$(echo "$CASV" | cut -d, -f4)
		BW=$(echo "$CASV" | cut -d, -f5)
		decode_bw
		LBAND=$LBAND" (CA, Bandwidth $BW MHz)"
		CHANNEL="$CHANNEL, "$(echo "$CASV" | cut -d, -f2)
		PCI="$PCI, "$(echo "$CASV" | cut -d, -f7)
	done
fi

CNMP=$(echo "$OX" | grep -o "+CNMP:[ ]*[0-9]\{1,3\}" | grep -o "[0-9]\{1,3\}")
TEMP=$(echo "$OX" | grep -o "+CPMUTEMP:[ ]*[-0-9]\+" | grep -o "[-0-9]\{1,4\}")

if [ -n "$CNMP" ]; then
	case $CNMP in
	"2"|"55" )
		NETMODE="1" ;;
	"13" )
		NETMODE="3" ;;
	"14" )
		NETMODE="5" ;;
	"38" )
		NETMODE="7" ;;
	"71" )
		NETMODE="9" ;;
	"109" )
		NETMODE="8" ;;
	* )
		NETMODE="0" ;;
	esac
fi
if [ -n "$TEMP" ]; then
	TEMP=$(echo $TEMP)$(printf "\xc2\xb0")"C"
fi
MODTYPE="10"

{
	echo 'CSQ="'"$CSQ"'"'
	echo 'CSQ_PER="'"$CSQ_PER"'"'
	echo 'CSQ_RSSI="'"$CSQ_RSSI"'"'
	echo 'ECIO="'"$ECIO"'"'
	echo 'RSCP="'"$RSCP"'"'
	echo 'ECIO1="'"$ECIO1"'"'
	echo 'RSCP1="'"$RSCP1"'"'
	echo 'MODE="'"$MODE"'"'
	echo 'MODTYPE="'"$MODTYPE"'"'
	echo 'NETMODE="'"$NETMODE"'"'
	echo 'CHANNEL="'"$CHANNEL"'"'
	echo 'LBAND="'"$LBAND"'"'
	echo 'PCI="'"$PCI"'"'
	echo 'TEMP="'"$TEMP"'"'
	echo 'SINR="'"$SINR"'"'
}  > /tmp/signal$CURRMODEM.file
