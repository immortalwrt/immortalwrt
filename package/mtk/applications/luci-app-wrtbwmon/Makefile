#
# Copyright (C) 2020 xxx <xxx@xxx.com>
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-wrtbwmon
PKG_VERSION:=2.0.13

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=

LUCI_TITLE:=A Luci module that uses wrtbwmon to track bandwidth usage
LUCI_DEPENDS:=+wrtbwmon
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
