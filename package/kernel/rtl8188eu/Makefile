#
# Copyright (C) 2021 ImmortalWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8188eu
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_URL:=https://github.com/aircrack-ng/rtl8188eus.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-03-20
PKG_SOURCE_VERSION:=0958f294f90b49d6bad4972b14f90676e5d858d3
PKG_MIRROR_HASH:=dead457e82a8b19a7fa66d7981c5f675e0d8d2d9d297aba5443c793822c22a8f

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8188eu
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek 8188EU(S)/8188ETV support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8188eu.ko
  AUTOLOAD:=$(call AutoProbe,rtl8188eu)
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

EXTRA_CFLAGS:= \
	-DRTW_SINGLE_WIPHY \
	-DRTW_USE_CFG80211_STA_EVENT \
	-DCONFIG_IOCTL_CFG80211 \
	-DCONFIG_CONCURRENT_MODE \
	-DBUILD_OPENWRT

ifeq ($(CONFIG_BIG_ENDIAN),y)
EXTRA_CFLAGS += -DCONFIG_BIG_ENDIAN
else
EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN
endif

EXTRA_KCONFIG:= \
	CONFIG_RTL8188EU=m \
	USER_MODULE_NAME=rtl8188eu

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

$(eval $(call KernelPackage,rtl8188eu))
