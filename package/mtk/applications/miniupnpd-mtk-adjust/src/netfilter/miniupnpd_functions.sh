#! /bin/sh
# $Id: miniupnpd_functions.sh,v 1.2 2018/05/29 10:25:44 nanard Exp $

IP=$(which ip) || {
	echo "Can't find ip" >&2
	exit 1
}
if [ -z "$IPV6" ]; then
	IPTABLES=$(which iptables) || {
		echo "Can't find iptables" >&2
		exit 1
	}
	IP="$IP -4"
else
	IPTABLES=$(which ip6tables) || {
		echo "Can't find ip6tables" >&2
		exit 1
	}
	IP="$IP -6"
fi
IPTABLES="$IPTABLES -w"

CHAIN=MINIUPNPD
CLEAN=

while getopts ":c:i:f" opt; do
	case $opt in
		c)
			CHAIN=$OPTARG
			;;
		i)
			EXTIF=$OPTARG
			;;
		f)
			CLEAN=yes
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

if [ -n "$EXT" ]; then
	if [ -z "$EXTIF" ]; then
		EXTIF=$(LC_ALL=C $IP route | grep 'default' | sed -e 's/.*dev[[:space:]]*//' -e 's/[[:space:]].*//') || {
			echo "Can't find default interface" >&2
			exit 1
		}
	fi
	#if [ -z "$IPV6" ]; then
	#	EXTIP=$(LC_ALL=C $IP addr show $EXTIF | awk '/inet/ { print $2 }' | cut -d "/" -f 1)
	#fi
fi

FDIRTY=$(LC_ALL=C $IPTABLES -t filter -L -n | awk "/$CHAIN / {printf \$1}")
if [ -z "$IPV6" ]; then
	NDIRTY=$(LC_ALL=C $IPTABLES -t nat -L -n | awk "/$CHAIN / {printf \$1}")
	MDIRTY=$(LC_ALL=C $IPTABLES -t mangle -L -n | awk "/$CHAIN / {printf \$1}")
	NPDIRTY=$(LC_ALL=C $IPTABLES -t nat -L -n | awk "/$CHAIN-POSTROUTING / {printf \$1}")
fi
