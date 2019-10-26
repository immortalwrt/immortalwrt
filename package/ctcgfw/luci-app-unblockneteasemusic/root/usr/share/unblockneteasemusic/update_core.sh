#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

function check_if_already_running(){
	running_tasks="$(ps |grep "unblockneteasemusic" |grep "update_core" |grep -v "grep" |awk '{print $1}' |wc -l)"
	[ "${running_tasks}" -gt "2" ] && echo -e "\nA task is already running." >>/tmp/unblockneteasemusic.log && exit 2
}

function clean_log(){
	echo "" > /tmp/unblockneteasemusic.log
}

function check_latest_version(){
	latest_ver="$(curl -s https://github.com/nondanee/UnblockNeteaseMusic/commits/master |tr -d '\n' |grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g')"
	[ -z "${latest_ver}" ] && echo -e "\nFailed to check latest version, please try again later." >>/tmp/unblockneteasemusic.log && exit 1
	if [ ! -e "/usr/share/unblockneteasemusic/local_ver" ]; then
		clean_log
		echo -e "Local version: NOT FOUND, cloud version: ${latest_ver}." >>/tmp/unblockneteasemusic.log
		update_core
	else
		if [ "$(cat /usr/share/unblockneteasemusic/local_ver)" != "${latest_ver}" ]; then
			clean_log
			echo -e "Local version: $(cat /usr/share/unblockneteasemusic/local_ver 2>/dev/null), cloud version: ${latest_ver}." >>/tmp/unblockneteasemusic.log
			update_core
		else
			echo -e "\nLocal version: $(cat /usr/share/unblockneteasemusic/local_ver 2>/dev/null), cloud version: ${latest_ver}." >>/tmp/unblockneteasemusic.log
			echo -e "You're already using the latest version." >>/tmp/unblockneteasemusic.log
			exit 3
		fi
	fi
}

function update_core(){
	echo -e "Updating core..." >>/tmp/unblockneteasemusic.log

	mkdir -p "/usr/share/unblockneteasemusic/core" >/dev/null 2>&1
	rm -rf /usr/share/unblockneteasemusic/core/* >/dev/null 2>&1

	curl -L "https://github.com/nondanee/UnblockNeteaseMusic/archive/master.tar.gz" -o "/usr/share/unblockneteasemusic/core/core.tar.gz" >/dev/null 2>&1
	tar -zxf "/usr/share/unblockneteasemusic/core/core.tar.gz" -C "/usr/share/unblockneteasemusic/core/" >/dev/null 2>&1
	mv /usr/share/unblockneteasemusic/core/UnblockNeteaseMusic-master/* "/usr/share/unblockneteasemusic/core/"
	rm -rf "/usr/share/unblockneteasemusic/core/core.tar.gz" "/usr/share/unblockneteasemusic/core/UnblockNeteaseMusic-master" >/dev/null 2>&1

	if [ ! -e "/usr/share/unblockneteasemusic/core/app.js" ]; then
		echo -e "Failed to download core." >>/tmp/unblockneteasemusic.log
		exit 1
	else
		[ "${luci_update}" == "y" ] && touch "/usr/share/unblockneteasemusic/update_successfully"
		echo -e "${latest_ver}" > /usr/share/unblockneteasemusic/local_ver
		/etc/init.d/unblockneteasemusic restart
	fi

	echo -e "Succeeded in updating core." >/tmp/unblockneteasemusic.log
	echo -e "Local version: $(cat /usr/share/unblockneteasemusic/local_ver 2>/dev/null), cloud version: ${latest_ver}.\n" >>/tmp/unblockneteasemusic.log
}

function main(){
	check_if_already_running
	check_latest_version
}

	[ "$1" == "luci_update" ] && luci_update="y"
	main
