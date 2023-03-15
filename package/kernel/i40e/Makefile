# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=i40e
PKG_VERSION:=2.22.18
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/e1000/i40e%20stable/$(PKG_VERSION)/
PKG_HASH:=ba3fed07b178bc78bfeab3ba309f812243cbc88032333edca891eb74022468f6

PKG_MAINTAINER:=Tianling Shen <cnsztl@immortalwrt.org>
PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=COPYING

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define KernelPackage/i40e-vendor
  SUBMENU:=Network Devices
  TITLE:=Intel(R) Ethernet Controller XL710 Family support (vendor driver)
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)-$(BOARD)-$(PKG_RELEASE)
  DEPENDS:=@PCI_SUPPORT +kmod-mdio +kmod-ptp +kmod-libphy @TARGET_x86
  CONFLICTS:=kmod-i40e
  FILES:= \
	$(PKG_BUILD_DIR)/src/intel_auxiliary.ko \
	$(PKG_BUILD_DIR)/src/i40e.ko
  AUTOLOAD:=$(call AutoProbe,i40e)
endef

define KernelPackage/i40e-vendor/description
  Kernel modules for Intel(R) Ethernet Controller XL710 Family 40 Gigabit Ethernet adapters.
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

$(eval $(call KernelPackage,i40e-vendor))
