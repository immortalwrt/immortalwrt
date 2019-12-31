#!/bin/sh
if [ -f /usr/share/clash/new_clashr_core_version ];then
rm -rf /usr/share/clash/new_clashr_core_version
fi
new_clashr_core_version=`wget -qO- "https://github.com/frainzy1477/clashrdev/tags"| grep "/frainzy1477/clashrdev/releases/tag/"| head -n 1| awk -F "/tag/v" '{print $2}'| sed 's/\">//'`
if [ $new_clashr_core_version ]; then
echo $new_clashr_core_version > /usr/share/clash/new_clashr_core_version 2>&1 & >/dev/null
elif [ $new_clashr_core_version =="" ]; then
echo 0 > /usr/share/clash/new_clashr_core_version 2>&1 & >/dev/null
fi
 
