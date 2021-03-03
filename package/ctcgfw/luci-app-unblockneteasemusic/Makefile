#
# Copyright (C) 2021 ImmortalWrt
# (https://immortalwrt.org)
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for UnblockNeteaseMusic
LUCI_DEPENDS:=+bash +busybox +coreutils +coreutils-nohup +curl +dnsmasq-full +ipset +libopenssl +node
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-unblockneteasemusic
PKG_VERSION:=2.8
PKG_RELEASE:=10

PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
