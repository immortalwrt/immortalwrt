#!/bin/sh

#指定文件路径
FILE="/etc/sysctl.d/qca-nss-ecm.conf"

#修改最大连接数
sed -i "s/nf_conntrack_max=.*/nf_conntrack_max=65535/g" $FILE

exit 0