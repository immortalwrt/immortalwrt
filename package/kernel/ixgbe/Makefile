# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=ixgbe
PKG_VERSION:=5.19.6
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/e1000/ixgbe%20stable/$(PKG_VERSION)/
PKG_HASH:=a844f1fea8064e30b276792455c3b286c1d7af26731e8f865d4a4e9ed1dcf4ab

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

define Build/Configure
	( \
		pushd $(PKG_BUILD_DIR)/src ; \
		KSRC=$(LINUX_DIR) \
		OUT=kcompat_generated_defs.h \
		CONFFILE=$(LINUX_DIR)/include/generated/autoconf.h \
		bash kcompat-generator.sh ; \
		popd ; \
	)
endef

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/src \
		CONFIG_IXGBE=m \
		modules
endef

$(eval $(call KernelPackage,ixgbe-vendor))
