# Copyright (C) 2016 Openwrt.org
#
# This is a free software, use it under GNU General Public License v3.0.
#
# Created By [CTCGFW]Project-OpenWrt
# https://github.com/project-openwrt

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for UnblockNeteaseMusic
LUCI_DEPENDS:=+bash +busybox +coreutils-nohup +curl +dnsmasq-full +ipset +libopenssl +node
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-unblockneteasemusic
PKG_VERSION:=2.8
PKG_RELEASE:=1

PKG_MAINTAINER:=[CTCGFW]Project-OpenWrt

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
