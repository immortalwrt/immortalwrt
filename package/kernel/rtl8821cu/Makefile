# SDPX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8821cu
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/brektrou/rtl8821CU.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-04-09
PKG_SOURCE_VERSION:=8c2226a74ae718439d56248bd2e44ccf717086d5
PKG_MIRROR_HASH:=3353a3bc4c73652eceaab95809aab27c3ae8bbf7e915b73488361a061ba91b84

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8821cu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8811CU/RTL8821CU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8821cu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8821cu)
  PROVIDES:=kmod-rtl8821cu
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

ifeq ($(BOARD),x86)
  NOSTDINC_FLAGS+= -mhard-float
endif

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8821CU=m \
		$(if $(CONFIG_aarch64),CONFIG_MP_VHT_HW_TX_MODE=n) \
		USER_MODULE_NAME=rtl8821cu \
		modules
endef

$(eval $(call KernelPackage,rtl8821cu))
