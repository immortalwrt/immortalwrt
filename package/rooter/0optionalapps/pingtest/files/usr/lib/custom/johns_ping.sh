#!/bin/sh

. /lib/functions.sh
 
ROOTER=/usr/lib/rooter
ROOTER_LINK="/tmp/links"

log() {
logger -t "Custom Ping Test " "$@"
}

tping() {
	hp=$(httping $2 -c 3 -s $1)
	pingg=$(echo $hp" " | grep -o "round-trip .\+ ms ")
	if [ -z "$pingg" ]; then
		tmp=0
	else
		tmp=200
	fi
}

uci set ping.ping.conn="4"
uci commit ping
	
CURRMODEM=1
CPORT=$(uci -q get modem.modem$CURRMODEM.commport)
DELAY=$(uci get ping.ping.delay)

TYPE=$(uci get ping.ping.type)
if [ $TYPE = "1" ]; then
log "Curl"
	RETURN_CODE_1=$(curl -m 10 -s -o /dev/null -w "%{http_code}" http://www.google.com/)
	RETURN_CODE_2=$(curl --ipv6 -m 10 -s -o /dev/null -w "%{http_code}" http://www.example.org/)
	RETURN_CODE_3=$(curl -m 10 -s -o /dev/null -w "%{http_code}" https://github.com)
else
log "Ping"
	tping "http://www.google.com/"; RETURN_CODE_1=$tmp
	tping "http://www.example.org/" "-6"; RETURN_CODE_2=$tmp
	tping "https://github.com"; RETURN_CODE_3=$tmp
fi

if [[ "$RETURN_CODE_1" != "200" &&  "$RETURN_CODE_2" != "200" &&  "$RETURN_CODE_3" != "200" ]]; then
	log "Bad Ping Test"
	if [ $TYPE = "1" ]; then
		tping "http://www.google.com/"; RETURN_CODE_1=$tmp
		tping "http://www.example.org/" "-6"; RETURN_CODE_2=$tmp
		tping "https://github.com"; RETURN_CODE_3=$tmp
	else
		RETURN_CODE_1=$(curl -m 10 -s -o /dev/null -w "%{http_code}" http://www.google.com/)
		RETURN_CODE_2=$(curl --ipv6 -m 10 -s -o /dev/null -w "%{http_code}" http://www.example.org/)
		RETURN_CODE_3=$(curl -m 10 -s -o /dev/null -w "%{http_code}" https://github.com)
	fi
	if [[ "$RETURN_CODE_1" != "200" &&  "$RETURN_CODE_2" != "200" &&  "$RETURN_CODE_3" != "200" ]]; then
		log "Second Bad Ping Test"
		uci set ping.ping.conn="3"
		uci commit ping
		ATCMDD="AT+CFUN=1,1"
		$ROOTER/gcom/gcom-locked "/dev/ttyUSB$CPORT" "run-at.gcom" "$CURRMODEM" "$ATCMDD"
		sleep $DELAY
		tries=0
		while [ $tries -lt 9 ]
		do
			CONN=$(uci -q get modem.modem$CURRMODEM.connected)
			if [ $CONN = "1" ]; then
				uci set ping.ping.conn="4"
				uci commit ping
				if [ $TYPE = "1" ]; then
				log "Curl"
					RETURN_CODE_1=$(curl -m 10 -s -o /dev/null -w "%{http_code}" http://www.google.com/)
					RETURN_CODE_2=$(curl --ipv6 -m 10 -s -o /dev/null -w "%{http_code}" http://www.example.org/)
					RETURN_CODE_3=$(curl -m 10 -s -o /dev/null -w "%{http_code}" https://github.com)
				else
				log "Ping"
					tping "http://www.google.com/"; RETURN_CODE_1=$tmp
					tping "http://www.example.org/" "-6"; RETURN_CODE_2=$tmp
					tping "https://github.com"; RETURN_CODE_3=$tmp
				fi
				if [[ "$RETURN_CODE_1" != "200" &&  "$RETURN_CODE_2" != "200" &&  "$RETURN_CODE_3" != "200" ]]; then
					uci set ping.ping.conn="1"
					uci commit ping
					reboot -f
				fi
				log "Second Ping Test Good"
				uci set ping.ping.conn="2"
				uci commit ping
				exit 0
			else
				sleep 20
				tries=$((tries+1))
			fi
		done
		reboot -f
	fi
else
	log "Good Ping"
	uci set ping.ping.conn="2"
	uci commit ping
fi
exit 0
