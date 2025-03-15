#! /bin/sh
# $Id: iptables_removeall.sh,v 1.10 2017/04/21 11:16:09 nanard Exp $

EXT=1
. $(dirname "$0")/miniupnpd_functions.sh

#removing the MINIUPNPD chain for nat
if [ "$NDIRTY" = "${CHAIN}Chain" ]; then
	$IPTABLES -t nat -F $CHAIN
	#$IPTABLES -t nat -D PREROUTING -d $EXTIP -i $EXTIF -j $CHAIN
	$IPTABLES -t nat -D PREROUTING -i $EXTIF -j $CHAIN
	$IPTABLES -t nat -X $CHAIN
elif [ "$NDIRTY" = "Chain" ]; then
	$IPTABLES -t nat -F $CHAIN
	$IPTABLES -t nat -X $CHAIN
fi

#removing the MINIUPNPD chain for mangle
if [ "$MDIRTY" = "${CHAIN}Chain" ]; then
	$IPTABLES -t mangle -F $CHAIN
	$IPTABLES -t mangle -D PREROUTING -i $EXTIF -j $CHAIN
	$IPTABLES -t mangle -X $CHAIN
elif [ "$MDIRTY" = "Chain" ]; then
	$IPTABLES -t mangle -F $CHAIN
	$IPTABLES -t mangle -X $CHAIN
fi

#removing the MINIUPNPD chain for filter
if [ "$FDIRTY" = "${CHAIN}Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -D FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
	$IPTABLES -t filter -X $CHAIN
elif [ "$FDIRTY" = "Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -X $CHAIN
fi

#removing the MINIUPNPD-POSTROUTING chain for nat
if [ "$NPDIRTY" = "${CHAIN}-POSTROUTINGChain" ]; then
	$IPTABLES -t nat -F $CHAIN-POSTROUTING
	$IPTABLES -t nat -D POSTROUTING -o $EXTIF -j $CHAIN-POSTROUTING
	$IPTABLES -t nat -X $CHAIN-POSTROUTING
elif [ "$NPDIRTY" = "Chain" ]; then
	$IPTABLES -t nat -F $CHAIN-POSTROUTING
	$IPTABLES -t nat -X $CHAIN-POSTROUTING
fi
