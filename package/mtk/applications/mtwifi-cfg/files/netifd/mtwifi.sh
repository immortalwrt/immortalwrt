#!/bin/sh
#
# Copyright (c) 2023, hanwckf <hanwckf@vip.qq.com>
#

. /lib/netifd/netifd-wireless.sh

init_wireless_driver "$@"

LOCK_FILE="/tmp/mtwifi.lock"

MTWIFI_MAX_AP_IDX=15
MTWIFI_MAX_APCLI_IDX=0
MTWIFI_CFG_IFNAME_KEY="mtwifi_ifname"

drv_mtwifi_init_device_config() {
	config_add_int txpower beacon_int dtim_period
	config_add_boolean mu_beamformer dbdc_main whnat bandsteering
	config_add_string country twt
}

drv_mtwifi_init_iface_config() {
	config_add_string 'ssid:string' macfilter bssid kicklow assocthres steeringthresold 
	config_add_boolean wmm hidden isolate ieee80211k ieee80211r
	config_add_int wpa_group_rekey frag rts
	config_add_array 'maclist:list(macaddr)' 'steeringbssid:list(macaddr)'
	config_add_boolean mumimo_dl mumimo_ul ofdma_dl ofdma_ul amsdu autoba uapsd
}

drv_mtwifi_cleanup() {
	return
}

mtwifi_vif_ap_config() {
	local name="$1"
	local ifname=""
	local disabled=""

	json_select config
	json_get_var disabled disabled
	json_select ..

	[ "$disabled" = "1" ] && return

	json_get_var ifname $MTWIFI_CFG_IFNAME_KEY

	if [ -n "$ifname" ]; then
		logger -t "netifd-mtwifi" "add $ifname to vifidx $name"
		wireless_add_vif "$name" "$ifname"
	fi
}

mtwifi_vif_sta_config() {
	local name="$1"
	local ifname=""
	local disabled=""

	json_select config
	json_get_var disabled disabled
	json_select ..

	[ "$disabled" = "1" ] && return

	json_get_var ifname $MTWIFI_CFG_IFNAME_KEY

	if [ -n "$ifname" ]; then
		logger -t "netifd-mtwifi" "add $ifname to vifidx $name"
		wireless_add_vif "$name" "$ifname"
	fi
}

mtwifi_vif_ap_set_data() {
	local ifname=""

	if [ ! $AP_IDX -gt $MTWIFI_MAX_AP_IDX ]; then
		ifname="${MTWIFI_AP_IF_PREFIX}${AP_IDX}"
		AP_IDX=$((AP_IDX+1))
	fi

	json_add_string "$MTWIFI_CFG_IFNAME_KEY" "$ifname"
}

mtwifi_vif_sta_set_data() {
	local ifname=""

	if [ ! $APCLI_IDX -gt $MTWIFI_MAX_APCLI_IDX ]; then
		ifname="${MTWIFI_APCLI_IF_PREFIX}${APCLI_IDX}"
		APCLI_IDX=$((APCLI_IDX+1))
	fi

	json_add_string "$MTWIFI_CFG_IFNAME_KEY" "$ifname"
}

drv_mtwifi_setup() {
	ubus -t 120 wait_for network.interface.lan

	local dev="$1"

	json_add_string device "$dev"

	lock $LOCK_FILE

	logger -t "netifd-mtwifi" "up: $dev"

	MTWIFI_AP_IF_PREFIX="$(l1util get $dev ext_ifname)"
	MTWIFI_APCLI_IF_PREFIX="$(l1util get $dev apcli_ifname)"

	AP_IDX=0
	for_each_interface ap mtwifi_vif_ap_set_data

	APCLI_IDX=0
	for_each_interface sta mtwifi_vif_sta_set_data

	json_dump | /sbin/mtwifi_cfg setup

	for_each_interface ap mtwifi_vif_ap_config
	for_each_interface sta mtwifi_vif_sta_config

	wireless_set_up

	lock -u $LOCK_FILE
}

drv_mtwifi_teardown() {
	local dev="$1"

	lock $LOCK_FILE

	logger -t "netifd-mtwifi" "down: $dev"

	/sbin/mtwifi_cfg down "$dev"

	lock -u $LOCK_FILE
}

add_driver mtwifi
