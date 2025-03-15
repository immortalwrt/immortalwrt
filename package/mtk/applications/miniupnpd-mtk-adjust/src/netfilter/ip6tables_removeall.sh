#! /bin/sh
# $Id: ip6tables_removeall.sh,v 1.1 2012/04/24 22:13:41 nanard Exp $

IPV6=1
EXT=1
. $(dirname "$0")/miniupnpd_functions.sh

#removing the MINIUPNPD chain for filter
if [ "$FDIRTY" = "${CHAIN}Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -D FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
	$IPTABLES -t filter -X $CHAIN
elif [ "$FDIRTY" = "Chain" ]; then
	$IPTABLES -t filter -F $CHAIN
	$IPTABLES -t filter -X $CHAIN
fi
