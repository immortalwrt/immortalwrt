#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=Luci for JD dailybonus Script
LUCI_DEPENDS:=+libuci-lua +lua +node +node-request +wget-ssl
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-jd-dailybonus
PKG_VERSION:=1.0.5
PKG_RELEASE:=20210316

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
