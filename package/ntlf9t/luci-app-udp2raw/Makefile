#
# Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for udp2raw-tunnel
LUCI_DESCRIPTION:=LuCI Support for udp2raw-tunnel.
LUCI_DEPENDS:=+udp2raw-tunnel
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-udp2raw
PKG_VERSION:=1.0.0
PKG_RELEASE:=6

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
