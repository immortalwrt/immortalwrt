#
# Copyright (C) 2018 Hao Dong <halbertdong@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for speederv2-tunnel
LUCI_DEPENDS:=+speederv2-tunnel
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-speederv2
PKG_VERSION:=1.1.0
PKG_RELEASE:=3

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Hao Dong <halbertdong@gmail.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
