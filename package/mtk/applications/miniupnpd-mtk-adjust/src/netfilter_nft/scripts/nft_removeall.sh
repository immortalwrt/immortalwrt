#!/bin/sh
#
# Undo the things nft_init.sh did
#
# Do not disturb other existing structures in nftables, e.g. those created by firewalld
#

. "$(dirname "$0")/miniupnpd_functions.sh"

$NFT --check list table inet $TABLE > /dev/null 2>&1
if [ $? -eq "0" ]
then
	# then remove the table itself
	echo "Remove miniupnpd table"
	$NFT delete table inet $TABLE
fi

if [ "$TABLE" != "$NAT_TABLE" ]
then
	$NFT --check list table inet $NAT_TABLE > /dev/null 2>&1
	if [ $? -eq "0" ]; then
		# then remove the table itself
		echo "Remove miniupnpd nat table"
		$NFT delete table inet $NAT_TABLE
	fi
fi
