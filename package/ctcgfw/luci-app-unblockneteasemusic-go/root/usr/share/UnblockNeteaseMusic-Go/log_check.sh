#!/bin/bash
# Created By [CTCGFW]Project OpenWRT
# https://github.com/project-openwrt

log_max_size="4" #使用KB计算
log_file="/tmp/unblockneteasemusic-go.log"

(( log_size = "$(ls -l "${log_file}" | awk -F ' ' '{print $5}')" / "1024" ))
(( "${log_size}" >= "${log_max_size}" )) && echo "" > "${log_file}"

[ "*$(uci get unblockneteasemusic-go.@unblockneteasemusic-go[0].daemon_enable)*" == "*1*" ] && { [ -z "$(ps |grep "UnblockNeteaseMusic-Go" |grep -v "grep")" ] && /etc/init.d/unblockneteasemusic-go restart; }