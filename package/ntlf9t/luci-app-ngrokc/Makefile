#
# Copyright (C) 2015-2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for ngrokc.
LUCI_DESCRIPTION:=LuCI Support for ngrokc.
LUCI_DEPENDS:=+ngrokc
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-ngrokc
PKG_VERSION:=1.3
PKG_RELEASE:=1

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
