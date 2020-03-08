#
# Copyright 2020 Weizheng Li <lwz322@qq.com>
# Licensed to the public under the MIT License.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-frps
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_MAINTAINER:=Weizheng Li <lwz322@qq.com>

LUCI_TITLE:=LuCI support for Frps
LUCI_PKGARCH:=all
LUCI_DEPENDS:=+frps

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
