#!/bin/sh

#指定文件路径
FILE="/usr/share/rpcd/ucode/luci"

#添加NSS状态显示
sed -i "s#const fd = popen('top.*')#const fd = popen('/sbin/cpuusage')#g" $FILE

exit 0