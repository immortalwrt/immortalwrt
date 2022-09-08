#!/bin/sh

ROOTER_LINK="/tmp/links"

log() {
	logger -t "Reconnect Modem" "$@"
}

CURRMODEM=$1
log "Re-starting Connection for Modem $CURRMODEM"
$ROOTER_LINK/create_proto$CURRMODEM $CURRMODEM 1

