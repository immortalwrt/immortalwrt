#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 openwrt-brook-tproxy
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-brook
PKG_VERSION:=1.0.2
PKG_RELEASE:=3

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=WouldChar

PKG_BUILD_PARALLEL:=1

LUCI_TITLE:=LuCI Support for Brook.
LUCI_DEPENDS:=+ipset +ip +iptables-mod-tproxy +dnsmasq-full +coreutils +coreutils-base64 +brook

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/luci/luci.mk

$(eval $(call BuildPackage,$(PKG_NAME)))
