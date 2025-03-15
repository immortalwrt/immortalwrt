#! /bin/sh
# $Id: ip6tables_init_and_clean.sh,v 1.1 2012/04/24 22:13:41 nanard Exp $
# Improved Miniupnpd iptables init script.
# Checks for state of filter before doing anything..

IPV6=1
EXT=1
. $(dirname "$0")/miniupnpd_functions.sh
# -I inserts the rule at the head of the chain,
# -A appends the rule at the end of the chain
ADDCMD=-I
#ADDCMD=-A

if [ "$FDIRTY" = "${CHAIN}Chain" ]; then
	echo "Filter table dirty; Cleaning..."
elif [ "$FDIRTY" = "Chain" ]; then
	echo "Dirty filter chain but no reference..? Fixing..."
	$IPTABLES -t filter $ADDCMD FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
else
	echo "Filter table clean..initalizing.."
	$IPTABLES -t filter -N $CHAIN
	$IPTABLES -t filter $ADDCMD FORWARD -i $EXTIF ! -o $EXTIF -j $CHAIN
fi
if [ "$CLEAN" = "yes" ]; then
	$IPTABLES -t filter -F $CHAIN
fi
