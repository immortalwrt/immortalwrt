#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

function check_core_if_already_running(){
	running_tasks="$(ps |grep "unblockneteasemusic" |grep "update" |grep "core" |grep -v "grep" |awk '{print $1}' |wc -l)"
	[ "${running_tasks}" -gt "2" ] && echo -e "\nA task is already running." >>/tmp/unblockneteasemusic.log && exit 2
}

function check_luci_if_already_running(){
	running_tasks="$(ps |grep "unblockneteasemusic" |grep "update" |grep "update_luci" |grep -v "grep" |awk '{print $1}' |wc -l)"
	[ "${running_tasks}" -gt "2" ] && echo -e "\nA task is already running." >>/tmp/unblockneteasemusic.log && exit 2
}

function clean_log(){
	echo "" > /tmp/unblockneteasemusic.log
}

function check_luci_latest_version(){
	luci_latest_ver="$(curl -s 'https://github.com/project-openwrt/luci-app-unblockneteasemusic/releases/latest'| grep -Eo '[0-9\.]+\-[0-9]+')"
	[ -z "${luci_latest_ver}" ] && echo -e "\nFailed to check latest LuCI version, please try again later." >>/tmp/unblockneteasemusic.log && exit 1
	if [ "$(opkg info 'luci-app-unblockneteasemusic' |sed -n '2p' |tr -d 'Version: ')" != "${luci_latest_ver}" ]; then
		clean_log
		echo -e "Local LuCI version: $(opkg info 'luci-app-unblockneteasemusic' |sed -n '2p' |tr -d 'Version: '), cloud LuCI version: ${luci_latest_ver}." >>/tmp/unblockneteasemusic.log
		update_luci
	else
		echo -e "\nLocal LuCI version: $(opkg info 'luci-app-unblockneteasemusic' |sed -n '2p' |tr -d 'Version: '), cloud LuCI version: ${luci_latest_ver}." >>/tmp/unblockneteasemusic.log
		echo -e "You're already using the latest LuCI version." >>/tmp/unblockneteasemusic.log
		exit 3
	fi
}

function update_luci(){
	echo -e "Updating LuCI..." >>/tmp/unblockneteasemusic.log

	mkdir -p "/tmp" >/dev/null 2>&1

	curl -sL "https://github.com/project-openwrt/luci-app-unblockneteasemusic/releases/download/v${luci_latest_ver}/luci-app-unblockneteasemusic_${luci_latest_ver}_all.ipk" -o "/tmp/luci-app-unblockneteasemusic_${luci_latest_ver}_all.ipk" >/dev/null 2>&1
	opkg install "/tmp/luci-app-unblockneteasemusic_${luci_latest_ver}_all.ipk"
	rm -f "/tmp/luci-app-unblockneteasemusic_${luci_latest_ver}_all.ipk"
	rm -rf "/tmp/luci-indexcache" "/tmp/luci-modulecache"

	if [ "$(opkg info 'luci-app-unblockneteasemusic' |sed -n '2p' |tr -d 'Version: ')" != "$1" ]; then
		echo -e "Failed to update LuCI." >>/tmp/unblockneteasemusic.log
		exit 1
	else
		touch "/usr/share/unblockneteasemusic/update_luci_successfully"
	fi

	echo -e "Succeeded in updating LuCI." >/tmp/unblockneteasemusic.log
	echo -e "Current LuCI version: ${luci_latest_ver}.\n" >>/tmp/unblockneteasemusic.log
}

function check_core_latest_version(){
	core_latest_ver="$(curl -s https://github.com/nondanee/UnblockNeteaseMusic/commits/master |tr -d '\n' |grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g')"
	[ -z "${core_latest_ver}" ] && echo -e "\nFailed to check latest core version, please try again later." >>/tmp/unblockneteasemusic.log && exit 1
	if [ ! -e "/usr/share/unblockneteasemusic/core_local_ver" ]; then
		clean_log
		echo -e "Local version: NOT FOUND, cloud version: ${core_latest_ver}." >>/tmp/unblockneteasemusic.log
		update_core
	else
		if [ "$(cat /usr/share/unblockneteasemusic/core_local_ver)" != "${core_latest_ver}" ]; then
			clean_log
			echo -e "Local core version: $(cat /usr/share/unblockneteasemusic/core_local_ver 2>/dev/null), cloud core version: ${core_latest_ver}." >>/tmp/unblockneteasemusic.log
			update_core
		else
			echo -e "\nLocal core version: $(cat /usr/share/unblockneteasemusic/core_local_ver 2>/dev/null), cloud core version: ${core_latest_ver}." >>/tmp/unblockneteasemusic.log
			echo -e "You're already using the latest core version." >>/tmp/unblockneteasemusic.log
			exit 3
		fi
	fi
}

function update_core(){
	echo -e "Updating core..." >>/tmp/unblockneteasemusic.log

	mkdir -p "/usr/share/unblockneteasemusic/core" >/dev/null 2>&1
	rm -rf /usr/share/unblockneteasemusic/core/* >/dev/null 2>&1

	curl -sL "https://github.com/nondanee/UnblockNeteaseMusic/archive/master.tar.gz" -o "/usr/share/unblockneteasemusic/core/core.tar.gz" >/dev/null 2>&1
	tar -zxf "/usr/share/unblockneteasemusic/core/core.tar.gz" -C "/usr/share/unblockneteasemusic/core/" >/dev/null 2>&1
	mv /usr/share/unblockneteasemusic/core/UnblockNeteaseMusic-master/* "/usr/share/unblockneteasemusic/core/"
	rm -rf "/usr/share/unblockneteasemusic/core/core.tar.gz" "/usr/share/unblockneteasemusic/core/UnblockNeteaseMusic-master" >/dev/null 2>&1

	if [ ! -e "/usr/share/unblockneteasemusic/core/app.js" ]; then
		echo -e "Failed to download core." >>/tmp/unblockneteasemusic.log
		exit 1
	else
		[ "${update_core_from_luci}" == "y" ] && touch "/usr/share/unblockneteasemusic/update_core_successfully"
		echo -e "${core_latest_ver}" > /usr/share/unblockneteasemusic/core_local_ver
		[ "${non_restart}" != "y" ] && /etc/init.d/unblockneteasemusic restart
	fi

	echo -e "Succeeded in updating core." >/tmp/unblockneteasemusic.log
	echo -e "Current core version: ${core_latest_ver}.\n" >>/tmp/unblockneteasemusic.log
}

case "$1" in
	"update_luci")
		check_luci_if_already_running
		check_luci_latest_version
		;;
	"update_core")
		check_core_if_already_running
		check_core_latest_version
		;;
	"update_core_non_restart")
		non_restart="y"
		check_core_if_already_running
		check_core_latest_version
		;;
	"update_core_from_luci")
		update_core_from_luci="y"
		check_core_if_already_running
		check_core_latest_version
		;;
	*)
		echo -e "Usage: ./update.sh {update_luci|update_core}"
		;;
esac