#! /bin/sh
# $Id: iptables_display.sh,v 1.4 2011/05/16 12:11:37 nanard Exp $

. $(dirname "$0")/miniupnpd_functions.sh

#display all chains relative to miniupnpd
$IPTABLES -v -n -t nat -L PREROUTING
$IPTABLES -v -n -t nat -L $CHAIN
$IPTABLES -v -n -t nat -L POSTROUTING
$IPTABLES -v -n -t nat -L $CHAIN-POSTROUTING
$IPTABLES -v -n -t mangle -L PREROUTING
$IPTABLES -v -n -t mangle -L $CHAIN
$IPTABLES -v -n -t filter -L FORWARD
$IPTABLES -v -n -t filter -L $CHAIN
