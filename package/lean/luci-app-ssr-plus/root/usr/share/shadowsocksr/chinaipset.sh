#!/bin/sh
LOCK_FILE="/var/lock/ssr-chinaipset.lock"
[ -f "$LOCK_FILE" ] && exit 2
[ -f "$1" ] && china_ip=$1
touch "$LOCK_FILE"
ipset -! flush china 2>/dev/null
ipset -! -R <<-EOF || rm -f $LOCK_FILE && exit 1
	create china hash:net
	$(cat ${china_ip:=/etc/ssr/china_ssr.txt} | sed -e "s/^/add china /")
EOF
rm -f $LOCK_FILE
