#
# Copyright (C) 2021 Van Waholtz <vanwaholtz@gmail.com>
#
# This is free software, licensed under the Apache License, Version 2.0.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-wrtbwmon
PKG_VERSION:=1.6.3
PKG_RELEASE:=1

PKG_LICENSE:=Apache-2.0
PKG_MAINTAINER:=Van Waholtz <vanwaholtz@gmail.com>

LUCI_TITLE:=A Luci module that uses wrtbwmon to track bandwidth usage
LUCI_DEPENDS:=+@BUSYBOX_CONFIG_IP +iptables
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
