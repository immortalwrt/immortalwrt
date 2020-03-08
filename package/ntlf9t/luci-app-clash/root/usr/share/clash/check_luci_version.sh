#!/bin/sh
if [ -f /usr/share/clash/new_luci_version ];then
rm -rf /usr/share/clash/new_luci_version
fi
new_version=`wget -qO- "https://github.com/frainzy1477/luci-app-clash/tags"| grep "/frainzy1477/luci-app-clash/releases/tag/"| head -n 1| awk -F "/tag/v" '{print $2}'| sed 's/\">//'`
if [ $new_version ]; then
echo $new_version > /usr/share/clash/new_luci_version 2>&1 & >/dev/null
elif [ $new_version =="" ]; then
echo 0 > /usr/share/clash/new_luci_version 2>&1 & >/dev/null
fi
 