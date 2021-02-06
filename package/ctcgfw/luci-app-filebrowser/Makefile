#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for FileBrowser
LUCI_DEPENDS:=+filebrowser
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-filebrowser
PKG_VERSION:=snapshot
PKG_RELEASE:=118071b

PKG_LICENSE:=GPLv3

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
