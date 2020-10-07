#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

NAME="unblockneteasemusic"

log_max_size="4" #使用KB计算
log_file="/tmp/$NAME.log"

(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
(( "${log_size}" >= "${log_max_size}" )) && echo "" > "${log_file}"

[ "*$(uci get $NAME.@$NAME[0].daemon_enable 2>/dev/null)*" == "*1*" ] && { [ -z "$(ps |grep "$NAME" |grep "app.js" |grep -v "grep")" ] && /etc/init.d/$NAME restart; }
