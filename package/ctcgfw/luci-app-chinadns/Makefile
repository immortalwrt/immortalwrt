#
# Copyright (C) 2017 Ian Li <OpenSource@ianli.xyz>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for ChinaDNS
LUCI_DESCRIPTION:=LuCI Support for ChinaDNS.
LUCI_DEPENDS:=+ChinaDNS +curl
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-chinadns
PKG_VERSION:=2.0.1
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Ian Li <OpenSource@ianli.xyz>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature

