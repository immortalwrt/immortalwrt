#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

commit_info="$(curl -s https://github.com/nondanee/UnblockNeteaseMusic/commits/master)"
latest_ver="$(echo -e "${commit_info}" |tr '\n' ' ' | grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g')"
latest_ver_mini="$(echo -e "${commit_info}" |tr -d '\n' | grep -Eo 'BtnGroup-item">      [0-9a-z]+' |sed -n 1p |sed 's#BtnGroup-item">      ##g')"

[ -z "${commit_info}" ] && echo "无法检测最新版本，请稍后重试" && exit 1
[ -z "${latest_ver}" ] && echo "无法检测最新版本，请稍后重试" && exit 1
[ -z "${latest_ver_mini}" ] && echo "无法检测最新版本，请稍后重试" && exit 1

if [ ! -e "/usr/share/unblockneteasemusic/local_ver" ]; then
	echo "最新版本：${latest_ver_mini}，点此更新"
else
	if [ "$(cat /usr/share/unblockneteasemusic/local_ver)" != "${latest_ver}" ]; then
		echo "最新版本：${latest_ver_mini}，点此更新"
	else
		echo "目前已是最新版本，无需更新"
	fi
fi