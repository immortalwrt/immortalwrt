#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8821cu
PKG_VERSION:=5.8.1
PKG_RELEASE:=1
PKG_LICENSE:=GPLv3

PKG_SOURCE_URL:=https://github.com/brektrou/rtl8821CU.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-07-27
PKG_SOURCE_VERSION:=93b04bfcd293fdd7d98d5a7c964ae9416a40159c
PKG_MIRROR_HASH:=0b0c12c69efa680481abc3a7703a1a224f7fcd6271f8eb3c038fbaef02945bfa

PKG_MAINTAINER:=[CTCGFW] Project OpenWrt
PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8821cu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtl8821cu Support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8821cu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8821cu)
  PROVIDES:=kmod-rtl8821cu
endef

define KernelPackage/rtl8821cu/description
 Kernel module for rtl8821cu wireless module.
endef

NOSTDINC_FLAGS = \
	-I$(PKG_BUILD_DIR) \
	-I$(PKG_BUILD_DIR)/include \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/autoconf.h \
	-include backport/backport.h

EXTRA_KCONFIG:=\
	CONFIG_RTL8821CU=m \
	USER_MODULE_NAME=rtl8821cu

ifeq ($(ARCH),aarch64)
	EXTRA_KCONFIG += CONFIG_MP_VHT_HW_TX_MODE=n
endif

EXTRA_CFLAGS:= \
	-DRTW_SINGLE_WIPHY \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_RADIO_WORK \
	-DCONFIG_IOCTL_CFG80211 \
	-DCONFIG_CONCURRENT_MODE \
	-DBUILD_OPENWRT \

ifeq ($(ARCH),arm)
	EXTRA_CFLAGS += -mfloat-abi=softfp
endif

ifeq ($(ARCH),x86_64)
	EXTRA_CFLAGS += -mhard-float
endif

MAKE_OPTS:= \
	$(KERNEL_MAKE_FLAGS) \
	M="$(PKG_BUILD_DIR)" \
	NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
	USER_EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

$(eval $(call KernelPackage,rtl8821cu))
