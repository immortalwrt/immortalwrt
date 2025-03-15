#! /bin/sh
# $Id: ip6tables_display.sh,v 1.1 2012/04/24 22:13:41 nanard Exp $

IPV6=1
. $(dirname "$0")/miniupnpd_functions.sh

#display all chains relative to miniupnpd
$IPTABLES -v -n -t filter -L FORWARD
$IPTABLES -v -n -t filter -L $CHAIN
