# SPDX-Identifier-License: GPL-3.0-only
#
# Copyright (C) 2018 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_LICENSE:=GPL-3.0-only

LUCI_TITLE:=LuCI support for MTK HNAT
LUCI_DEPENDS:=+kmod-tcp-bbr @!PACKAGE_luci-app-turboacc
LUCI_PKGARCH:=all

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
