# Copyright (C) 2018 XiaoShan mivm.cn

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI Support for k3usb3.0 on or off
LUCI_DESCRIPTION:=LuCI Support for k3usb3.0 on or off.
LUCI_DEPENDS:=@TARGET_bcm53xx_DEVICE_phicomm-k3
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-k3usb
PKG_VERSION:=1.1.0
PKG_RELEASE:=2

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Shenzt <shenzt@gmail.com>

include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
