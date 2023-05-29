# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8192eu
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://github.com/Mange/rtl8192eu-linux-driver.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-12-26
PKG_SOURCE_VERSION:=aeed529c77282cc0d8a2b6eb29186711e25555ce
PKG_MIRROR_HASH:=83a158ecdc9c1642d4fbad9647d6b6853067a91a25b499a1a7dd719b556ccbe5

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192eu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8192EU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core
  FILES:=$(PKG_BUILD_DIR)/rtl8192eu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8192eu)
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

ifeq ($(CONFIG_BIG_ENDIAN),y)
NOSTDINC_FLAGS += -DCONFIG_BIG_ENDIAN
else
NOSTDINC_FLAGS += -DCONFIG_LITTLE_ENDIAN
endif

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8192EU=m \
		USER_MODULE_NAME=rtl8192eu \
		modules
endef

$(eval $(call KernelPackage,rtl8192eu))
