#!/bin/sh

. "$(dirname "$0")/miniupnpd_functions.sh"

# Prerouting
$NFT list chain inet $NAT_TABLE $PREROUTING_CHAIN
# Postrouting
$NFT list chain inet $NAT_TABLE $POSTROUTING_CHAIN
# Filter
$NFT list chain inet $TABLE $CHAIN
