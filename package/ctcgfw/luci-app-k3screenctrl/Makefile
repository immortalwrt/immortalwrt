# Copyright (C) 2018 XiaoShan mivm.cn

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for k3screenctrl
LUCI_DESCRIPTION:=LuCI Support for k3screenctrl.
LUCI_DEPENDS:=+phicomm-k3screenctrl
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-k3screenctrl
PKG_VERSION:=1.1.0
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Hill <lufanzhong@gmail.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
