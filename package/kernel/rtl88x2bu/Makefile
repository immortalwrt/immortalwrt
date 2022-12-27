# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl88x2bu
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/morrownr/88x2bu-20210702.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-12-21
PKG_SOURCE_VERSION:=46e266521462a0cc40379debaa77c5fcc0bec258
PKG_MIRROR_HASH:=bf662d5e1904e49854ea40842168b80b6c36e9a47d0763f9e77bc1080a21c25d

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl88x2bu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek 8812BU/8822BU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl88x2bu.ko
  AUTOLOAD:=$(call AutoProbe,rtl88x2bu)
  PROVIDES:=kmod-rtl88x2bu
endef

NOSTDINC_FLAGS:= \
	$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/autoconf.h \
	-include backport/backport.h

NOSTDINC_FLAGS+= \
	-DRTW_SINGLE_WIPHY \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_IOCTL_CFG80211 \
	-DCONFIG_CONCURRENT_MODE \
	-DBUILD_OPENWRT

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8822BU=m \
		USER_MODULE_NAME=rtl88x2bu \
		modules
endef

$(eval $(call KernelPackage,rtl88x2bu))
