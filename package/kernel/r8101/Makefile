# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=r8101
PKG_VERSION:=1.038.02
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=@IMMORTALWRT
PKG_HASH:=4141ebda97e277b5807a98b4090d7884d4463ac7a048676c72cb648ae4e8ae9e
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/r8101
  TITLE:=Driver for Realtek r8101 chipsets
  SUBMENU:=Network Devices
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT
  FILES:= $(PKG_BUILD_DIR)/src/r8101.ko
  AUTOLOAD:=$(call AutoProbe,r8101)
endef

define KernelPackage/r8101/description
  This package contains a driver for Realtek r8101 chipsets.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) M=$(PKG_BUILD_DIR)/src modules
endef

$(eval $(call KernelPackage,r8101))
