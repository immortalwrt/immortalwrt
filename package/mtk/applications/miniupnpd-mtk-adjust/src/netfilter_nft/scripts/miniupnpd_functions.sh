#! /bin/sh

NFT=$(which nft) || {
	echo "Can't find nft" >&2
	exit 1
}

TABLE="filter"
NAT_TABLE="filter"
CHAIN="miniupnpd"
PREROUTING_CHAIN="prerouting_miniupnpd"
POSTROUTING_CHAIN="postrouting_miniupnpd"

while getopts ":t:n:c:p:r:" opt; do
	case $opt in
		t)
			TABLE=$OPTARG
			;;
		n)
			NAT_TABLE=$OPTARG
			;;
		c)
			CHAIN=$OPTARG
			;;
		p)
			PREROUTING_CHAIN=$OPTARG
			;;
		r)
			POSTROUTING_CHAIN=$OPTARG
			;;
		\?)
			echo "Invalid option: -$OPTARG" >&2
			exit 1
			;;
		:)
			echo "Option -$OPTARG requires an argument." >&2
			exit 1
			;;
	esac
done
