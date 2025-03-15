#!/bin/sh
#
# Copyright (C) 2023, hanwckf <hanwckf@vip.qq.com>
#

append DRIVERS "mtwifi"

detect_mtwifi() {
	local idx ifname
	local band hwmode htmode htbsscoex ssid dbdc_main channel
	if [ -d "/sys/module/mt_wifi" ]; then
		dev_list="$(l1util list)"
		for dev in $dev_list; do
			config_get type ${dev} type
			[ "$type" = "mtwifi" ] || {
				ifname="$(l1util get ${dev} main_ifname)"
				idx="$(l1util get ${dev} subidx)"
				if [ $idx -eq 1 ]; then
					band="2g"
					hwmode="11g"
					htmode="HE40"
					htbsscoex="1"
					ssid="ImmortalWrt-2.4G"
					dbdc_main="1"
					txpower="100"
					channel="auto"
				else
					band="5g"
					hwmode="11a"
					htmode="HE160"
					htbsscoex="0"
					ssid="ImmortalWrt-5G"
					channel="36"
					txpower="100"
					dbdc_main="0"
				fi
				uci -q batch <<-EOF
					set wireless.${dev}=wifi-device
					set wireless.${dev}.type=mtwifi
					set wireless.${dev}.phy=${ifname}
					set wireless.${dev}.hwmode=${hwmode}
					set wireless.${dev}.band=${band}
					set wireless.${dev}.dbdc_main=${dbdc_main}
					set wireless.${dev}.channel=${channel}
					set wireless.${dev}.txpower=${txpower}
					set wireless.${dev}.htmode=${htmode}
					set wireless.${dev}.country=CN
					set wireless.${dev}.mu_beamformer=1
					set wireless.${dev}.noscan=${htbsscoex}
					set wireless.${dev}.serialize=1
					
					set wireless.default_${dev}=wifi-iface
					set wireless.default_${dev}.device=${dev}
					set wireless.default_${dev}.network=lan
					set wireless.default_${dev}.mode=ap
					set wireless.default_${dev}.ssid=${ssid}
					set wireless.default_${dev}.encryption=none
EOF
				uci -q commit wireless
			}
		done
	fi
}

