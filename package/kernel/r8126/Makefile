# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2024 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8126
PKG_VERSION:=10.013.00
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@IMMORTALWRT
PKG_HASH:=b41bda6ff3bbb7d9bc5b81c5c21355f031587d3a3a5862abcd4d766e942272e7

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8126
  TITLE:=Driver for Realtek r8126 chipsets
  SUBMENU:=Network Devices
  DEPENDS:=@PCI_SUPPORT
  FILES:= $(PKG_BUILD_DIR)/src/r8126.ko
  AUTOLOAD:=$(call AutoProbe,r8126)
endef

define KernelPackage/r8126/description
  This package contains a driver for Realtek r8126 chipsets.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/src \
		CONFIG_ASPM=n \
		ENABLE_RSS_SUPPORT=y \
		modules
endef

$(eval $(call KernelPackage,r8126))
