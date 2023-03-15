# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ixgbevf
PKG_VERSION:=4.17.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/e1000/ixgbevf%20stable/$(PKG_VERSION)/
PKG_HASH:=59813a6f92e699b4cb6983b70b91137eece4609a113fa7bb60db0be9889d2b46

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ixgbevf-vendor
  SUBMENU:=Network Devices
  TITLE:=Intel(R) 82599 Virtual Function Ethernet support (vendor driver)
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT +kmod-ixgbe-vendor
  CONFLICTS:=kmod-ixgbevf
  FILES:=$(PKG_BUILD_DIR)/src/ixgbevf.ko
  AUTOLOAD:=$(call AutoProbe,ixgbevf)
endef

define KernelPackage/ixgbevf-vendor/description
  Kernel modules for Intel(R) 82599 Virtual Function Ethernet adapters.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/src \
		CONFIG_IXGBEVF=m \
		modules
endef

$(eval $(call KernelPackage,ixgbevf-vendor))
