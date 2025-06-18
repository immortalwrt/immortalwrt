#!/bin/sh
. /usr/share/libubox/jshn.sh

# 默认WIFI设置
BASE_SSID='OWRT'
BASE_WORD='12345678'
BASE_POWER='auto'

# 获取无线设备的数量
RADIO_NUM=$(uci show wireless | grep -c "wifi-device")

# 如果没有找到无线设备，直接退出
[ "$RADIO_NUM" -eq 0 ] && exit 0

# 为部分机型设置发射功率
board_name=$(cat /tmp/sysinfo/board_name)
case "${board_name}" in
jdcloud,re-ss-01)
	BASE_POWER='20'
	;;
esac

# 配置参数
configure_wifi() {
	local radio=$1
	local channel=$2
	local htmode=$3
	local ssid=$4
	local current_encryption=$(uci get wireless.default_radio${radio}.encryption)

	# 如果当前加密方式已设置且不为"none"，则不更新配置
	if [ -n "$current_encryption" ] && [ "$current_encryption" != "none" ]; then
		echo "No update needed for radio${radio} with channel ${channel} and SSID ${ssid}"
		return 0
	fi

	# 设置无线设备参数
	uci set wireless.radio${radio}.channel=${channel}
	uci set wireless.radio${radio}.htmode=${htmode}
	uci set wireless.radio${radio}.txpower=${BASE_POWER}
	uci set wireless.radio${radio}.country='CN'
	uci set wireless.radio${radio}.disabled='0'
	uci set wireless.radio${radio}.mu_beamformer='1'

	uci set wireless.default_radio${radio}.ssid=${ssid}
	uci set wireless.default_radio${radio}.key=${BASE_WORD}
	uci set wireless.default_radio${radio}.encryption='psk2+ccmp'
	uci set wireless.default_radio${radio}.max_inactivity='3600'
	uci set wireless.default_radio${radio}.bss_transition='1'
	uci set wireless.default_radio${radio}.ieee80211k='1'
	uci set wireless.default_radio${radio}.time_advertisement='2'
	uci set wireless.default_radio${radio}.time_zone='CST-8'
	uci set wireless.default_radio${radio}.wnm_sleep_mode='1'
	uci set wireless.default_radio${radio}.wnm_sleep_mode_no_keys='1'
}

# 查询mode
query_mode() {
	json_load_file "/etc/board.json"
	json_select wlan
	json_get_keys phy_keys
	for phy in $phy_keys; do
		json_select $phy
		json_get_var path_value "path"
		if [ "$path_value" = "$1" ]; then
			json_select info
			json_select bands
			json_get_keys band_keys
			for band in $band_keys; do
				json_select $band
				json_select modes
				json_get_keys mode_keys
				for mode in $mode_keys; do
					json_get_var mode_value $mode
					last_mode=$mode_value
				done
				json_select ..
				json_select ..
			done
			echo "$last_mode"
			return
		fi
		json_select ..
	done
}

# 设置无线设备的默认配置
FIRST_5G=''
set_wifi_def_cfg() {
	local band=$(uci get wireless.radio${1}.band)
	local path=$(uci get wireless.radio${1}.path)
	local htmode=$(query_mode "$path")
	local channel=6
	local ssid="$BASE_SSID"

	case "$band" in
	'5g')
		channel=149
		[ "$htmode" = 'HE160' ] || [ "$htmode" = 'VHT160' ] && channel=44
		if [ -z "$FIRST_5G" ]; then
			[ "$RADIO_NUM" -gt 2 ] && ssid="${BASE_SSID}-5G_1" || ssid="${BASE_SSID}-5G"
			FIRST_5G=1
		else
			ssid="${BASE_SSID}-5G_2"
		fi
		;;
	*)
		case "$htmode" in
		'HT40' | 'VHT40' | 'HE40')
			htmode="${htmode%40}20"
			;;
		esac
		;;
	esac

	configure_wifi "$1" "$channel" "$htmode" "$ssid"
}

for i in $(seq 0 $((RADIO_NUM - 1))); do
	set_wifi_def_cfg "$i"
done

# 提交配置并重启网络服务
uci commit wireless

exit 0
