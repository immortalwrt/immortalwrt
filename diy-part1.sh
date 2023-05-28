#!/bin/bash
#
# Copyright (c) 2019-2020 P3TERX <https://p3terx.com>
#
# This is free software, licensed under the MIT License.
# See /LICENSE for more information.
#
# https://github.com/P3TERX/Actions-OpenWrt
# File name: diy-part1.sh
# Description: OpenWrt DIY script part 1 (Before Update feeds)
#

# Uncomment a feed source
#sed -i 's/^#\(.*helloworld\)/\1/' feeds.conf.default

# Add a feed source
#echo 'src-git helloworld https://github.com/fw876/helloworld' >>feeds.conf.default
echo 'src-git passwall https://github.com/xiaorouji/openwrt-passwall' >>feeds.conf.default
echo 'src-git 3ginfo https://github.com/4IceG/luci-app-3ginfo-lite.git' >>feeds.conf.default
echo 'src-git atc https://github.com/4IceG/luci-app-atcommands.git' >>feeds.conf.default
echo 'src-git  ATC https://github.com/4IceG/luci-app-atcommands.git' >>feeds.conf.default
echo 'src-git sms https://github.com/4IceG/luci-app-sms-tool.git' >>feeds.conf.default
echo 'src-git modemfeed https://github.com/koshev-msk/modemfeed.git' >>feeds.conf.default
