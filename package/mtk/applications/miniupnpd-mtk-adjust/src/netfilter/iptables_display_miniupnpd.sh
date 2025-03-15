#! /bin/sh
# $Id: iptables_display_miniupnpd.sh,v 1.1 2016/02/12 15:23:29 nanard Exp $

. $(dirname "$0")/miniupnpd_functions.sh

#display miniupnpd chains
$IPTABLES -v -n -t nat -L $CHAIN
$IPTABLES -v -n -t nat -L $CHAIN-POSTROUTING
$IPTABLES -v -n -t mangle -L $CHAIN
$IPTABLES -v -n -t filter -L $CHAIN
