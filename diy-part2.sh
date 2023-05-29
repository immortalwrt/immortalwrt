#!/bin/bash
#
# Copyright (c) 2019-2020 P3TERX <https://p3terx.com>
#
# This is free software, licensed under the MIT License.
# See /LICENSE for more information.
#
# https://github.com/P3TERX/Actions-OpenWrt
# File name: diy-part2.sh
# Description: OpenWrt DIY script part 2 (After Update feeds)
#

# Modify default IP
sed -i 's/192.168.1.1/192.168.1.1/g' package/base-files/files/bin/config_generate
sed -i '/CONFIG_PACKAGE_glib2=y/d' .config
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter-builds/0protocols/luci-proto-3x package/luci-proto-3x
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter-builds/0protocols/luci-proto-mbim package/luci-proto-mbim
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter/0drivers/rqmi package/rqmi
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter/0basicsupport/ext-sms package/ext-sms
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter/0basicsupport/ext-buttons package/ext-buttons
svn co https://github.com/ofmodemsandmen/ROOterSource2203/trunk/package/rooter/ext-rooter-basic package/ext-rooter-basic
svn co https://github.com/Roxy09099/ROOterSource2203/trunk/package/rooter/0drivers/rmbim package/rmbim
