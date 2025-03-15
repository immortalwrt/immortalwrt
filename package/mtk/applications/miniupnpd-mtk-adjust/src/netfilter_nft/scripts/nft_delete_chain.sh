#!/bin/sh

. "$(dirname "$0")/miniupnpd_functions.sh"

# Prerouting
$NFT delete chain inet $NAT_TABLE $PREROUTING_CHAIN
# Postrouting
$NFT delete chain inet $NAT_TABLE $POSTROUTING_CHAIN
# Filter
$NFT delete chain inet $TABLE $CHAIN
