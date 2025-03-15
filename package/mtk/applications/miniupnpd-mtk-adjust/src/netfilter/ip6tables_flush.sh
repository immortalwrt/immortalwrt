#! /bin/sh
# $Id: ip6tables_flush.sh,v 1.1 2012/04/24 22:13:41 nanard Exp $

IPV6=1
. $(dirname "$0")/miniupnpd_functions.sh

#flush all rules owned by miniupnpd
$IPTABLES -t filter -F $CHAIN
