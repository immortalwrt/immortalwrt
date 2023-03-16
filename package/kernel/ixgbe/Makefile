# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ixgbe
PKG_VERSION:=5.18.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/e1000/ixgbe%20stable/$(PKG_VERSION)/
PKG_HASH:=4ca9cc8d9041b8c9b4fdbbab4fe079e5da422fa1697ca5a58ebfa95d21e62daa

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/ixgbe-vendor
  SUBMENU:=Network Devices
  TITLE:=Intel(R) 82598/82599 PCI-Express 10 Gigabit Ethernet support (vendor driver)
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT +kmod-ptp +kmod-libphy @TARGET_x86
  CONFLICTS:=kmod-ixgbe
  FILES:=$(PKG_BUILD_DIR)/src/ixgbe.ko
  AUTOLOAD:=$(call AutoProbe,ixgbe)
endef

define KernelPackage/ixgbe-vendor/description
  Kernel modules for Intel(R) 82598/82599 PCI-Express 10 Gigabit Ethernet adapters.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/src \
		CONFIG_IXGBE=m \
		modules
endef

$(eval $(call KernelPackage,ixgbe-vendor))
