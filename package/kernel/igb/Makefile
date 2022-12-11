# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=igb
PKG_VERSION:=5.13.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@IMMORTALWRT
PKG_HASH:=4ccae73110f3c871a17b7cc6df7dbdf6295fc90f4941e8b2978a471c2fec83cb

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/igb-vendor
  SUBMENU:=Network Devices
  TITLE:=Intel(R) 82575/82576 PCI-Express Gigabit Ethernet support (vendor driver)
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT +kmod-i2c-core +kmod-i2c-algo-bit +kmod-ptp +kmod-hwmon-core
  CONFLICTS:=kmod-igb
  FILES:=$(PKG_BUILD_DIR)/src/igb.ko
  AUTOLOAD:=$(call AutoProbe,igb)
endef

define KernelPackage/igb-vendor/description
  Kernel modules for Intel(R) 82575/82576 PCI-Express Gigabit Ethernet adapters.
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/src \
		CONFIG_IGB=m \
		modules
endef

$(eval $(call KernelPackage,igb-vendor))
