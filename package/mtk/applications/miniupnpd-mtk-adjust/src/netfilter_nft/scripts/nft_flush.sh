#!/bin/sh

. "$(dirname "$0")/miniupnpd_functions.sh"

$NFT flush chain inet $TABLE $CHAIN
$NFT flush chain inet $NAT_TABLE $PREROUTING_CHAIN
$NFT flush chain inet $NAT_TABLE $POSTROUTING_CHAIN
