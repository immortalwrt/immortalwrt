#! /bin/sh
# $Id: iptables_flush.sh,v 1.6 2017/04/21 11:16:09 nanard Exp $

. $(dirname "$0")/miniupnpd_functions.sh

#flush all rules owned by miniupnpd
$IPTABLES -t nat -F $CHAIN
$IPTABLES -t nat -F $CHAIN-POSTROUTING
$IPTABLES -t filter -F $CHAIN
$IPTABLES -t mangle -F $CHAIN
