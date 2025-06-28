#!/bin/sh
# shellcheck disable=3037,3060,2034,1091,2166

# 检测终端并设置颜色
if [ -t 1 ]; then
	red="\033[31m"        # 红色
	green="\033[32m"      # 绿色
	yellow="\033[33m"     # 黄色
	blue="\033[34m"       # 蓝色
	magenta="\033[35m"    # 品红/紫色
	cyan="\033[36m"       # 青色
	white="\033[37m"      # 白色
	reset="\033[m"        # 重置
	bold="\033[1m"        # 加粗
fi

# 获取系统信息
device="$(jsonfilter -e '@.model.name' < /etc/board.json || echo "N/A")"
kernel="$(uname -r 2>/dev/null || echo "N/A")"
cpu_mode="$(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || echo "N/A")"
system="$(cat /etc/openwrt_release 2>/dev/null | awk -F "'" '/DESCRIPTION/ {print $2}' || echo "N/A")"

# 获取固件版本信息
nss_fw="$(grep -m1 -hao 'NSS.[^[:cntrl:]]*' /lib/firmware/qca-nss0.bin 2>/dev/null || echo "N/A")"
mac80211_fw="$(awk '/version/{print $NF;exit}' /lib/modules/*/compat.ko 2>/dev/null || echo "N/A")"

# 获取ATH11K固件信息
ath11k_fw=""
for file in /lib/firmware/IPQ*/q6_fw.b04 /lib/firmware/ath11k/QCN*/hw*/amss.bin; do
	[ -f "$file" ] || continue
	platform="$(echo "$file" | grep -Eo 'IPQ[0-9]+|QCN[0-9]+')"
	version="$(grep -m1 -hao 'WLAN.[^[:cntrl:]]*SILICONZ-[0-9]' "$file")"
	[ -n "$version" ] && ath11k_fw="${ath11k_fw}${platform} ${version}\n            "
done
ath11k_fw=${ath11k_fw:-"N/A"}

# 显示统计信息
echo -e "${bold}${magenta}    DEVICE${reset}: ${cyan}${device}${reset}"
echo -e "${bold}${magenta}    KERNEL${reset}: ${cyan}${kernel}${reset}"
echo -e "${bold}${magenta}  CPU MODE${reset}: ${cyan}${cpu_mode}${reset}"
echo -e "${bold}${magenta}    SYSTEM${reset}: ${yellow}${system}${reset}"
echo -e "${bold}${magenta}    NSS FW${reset}: ${blue}${nss_fw}${reset}"
echo -e "${bold}${magenta}  MAC80211${reset}: ${blue}${mac80211_fw}${reset}"
echo -e "${bold}${magenta} ATH11K FW${reset}: ${blue}${ath11k_fw}${reset}"

# 显示GRO分片状态
echo -ne "${bold}${magenta} INTERFACE${reset}: ${white}${reset}"
count=0
for iface in /sys/class/net/br-lan/device /sys/class/net/*/device; do
	iface=${iface%/*}
	iface=${iface##*/}
	ethtool -k "$iface" | awk -v count=$count -v face="$iface" -v rst="${reset}" -v red="${red}" -v green="${green}" '
	BEGIN { settings=""; if(count>0) space="            " }
	/tx-checksumming|rx-gro-list/ {
		color=green
		if($2=="off") color=red
		settings = settings $1 " " sprintf("%s%-3s%s", color,$2,rst) " ";
	}
	END { printf "%s%-11s%s\n", space, face, settings; }'
	count=$((count + 1))
done

# 显示NSS相关插件包
echo -ne "\n${bold}${magenta}  NSS PKGS${reset}: ${white}"
apk list -I | awk -v count=0 '
	/kmod-qca|^nss/ {
	if(count>0) space="            "
	print space $1
	count++
}'
