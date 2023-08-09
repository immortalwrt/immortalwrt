# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2023 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=iavf
PKG_VERSION:=4.9.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/e1000/iavf%20stable/$(PKG_VERSION)/
PKG_HASH:=6bf25bd7179b5bca1b10168f57ca4ce9fe54d08574e8d7613fad7c15800c7f71

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/iavf-vendor
  SUBMENU:=Network Devices
  TITLE:=Intel(R) Ethernet Adaptive Virtual Function support (vendor driver)
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT +kmod-i40e-vendor
  CONFLICTS:=kmod-iavf
  FILES:=$(PKG_BUILD_DIR)/src/iavf.ko
  AUTOLOAD:=$(call AutoProbe,iavf)
endef

define KernelPackage/iavf-vendor/description
  Kernel modules for Intel XL710,
          X710, X722, XXV710, and all devices advertising support for
          Intel Ethernet Adaptive Virtual Function devices.
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
		NEED_AUX_BUS=2 \
		modules
endef

$(eval $(call KernelPackage,iavf-vendor))
