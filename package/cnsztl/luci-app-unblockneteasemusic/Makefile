# Copyright (C) 2016 Openwrt.org
#
# This is a free software, use it under GNU General Public License v3.0.
#
# Created By [CTCGFW]Project-OpenWRT
# https://github.com/project-openwrt

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for UnblockNeteaseMusic-nondanee
LUCI_DEPENDS:=+UnblockNeteaseMusic-nondanee +bash +dnsmasq-full +ipset
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-unblockneteasemusic
PKG_VERSION:=1.5
PKG_RELEASE:=1

PKG_MAINTAINER:=<https://github.com/project-openwrt/UnblockNeteaseMusic>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
