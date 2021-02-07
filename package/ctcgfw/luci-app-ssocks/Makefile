# Copyright (C) 2016 Openwrt.org
#
# This is a free software, use it under GNU General Public License v3.0.
#
# Created By ImmortalWrt
# https://github.com/project-openwrt

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for sSocks
LUCI_DEPENDS:=+busybox +iptables +ssocksd
LUCI_PKGARCH:=all
PKG_NAME:=luci-app-ssocks
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_MAINTAINER:=ImmortalWrt

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
