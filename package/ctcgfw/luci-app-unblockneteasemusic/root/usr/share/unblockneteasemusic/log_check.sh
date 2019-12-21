#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

log_max_size="4" #使用KB计算
log_file="/tmp/unblockneteasemusic.log"

(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
(( "${log_size}" >= "${log_max_size}" )) && echo "" > "${log_file}"

[ "*$(uci get unblockneteasemusic.@unblockneteasemusic[0].daemon_enable 2>/dev/null)*" == "*1*" ] && { [ -z "$(ps |grep "unblockneteasemusic" |grep "app.js" |grep -v "grep")" ] && /etc/init.d/unblockneteasemusic restart; }