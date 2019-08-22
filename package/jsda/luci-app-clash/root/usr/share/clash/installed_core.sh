#!/bin/sh
if [ -f /usr/share/clash/installed_core ];then
rm -rf /usr/share/clash/installed_core
fi
check_core=$(opkg list-installed | grep 'clash' |awk -F ' - ' 'NR==1{print $1}' 2>/dev/null)
if [ $check_core == 'clash' ];then
curent_core=$(opkg list-installed | grep 'clash' |awk -F ' - ' 'NR==1{print $2}' 2>/dev/null)
echo $curent_core > /usr/share/clash/installed_core 2>&1 & >/dev/null
elif [ $check_core == 'luci-app-clash' ];then
echo 0 > /usr/share/clash/installed_core 2>&1 & >/dev/null
fi
