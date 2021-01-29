#
# Copyright (C) 2020 Marty Jones <mj8263788@gmail.com>
#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8812au-ac
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/aircrack-ng/rtl8812au.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2020-06-26
PKG_SOURCE_VERSION:=58f6be25f7c86d7ba67b0ccc638cf667755e6e1b
PKG_MIRROR_HASH:=60359df8b49fa433d38b968b0df7eaddaca10f13cdd57471394bac1f6e5a162e

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8812au-ac
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek rtl8812au/21au wireless drivers
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  FILES:=$(PKG_BUILD_DIR)/rtl8812au.ko
  AUTOLOAD:=$(call AutoProbe,rtl8812au)
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

NOSTDINC_FLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT \
	-DRTW_SINGLE_WIPHY -DCONFIG_CONCURRENT_MODE -D_LINUX_BYTEORDER_SWAB_H \
	-DBUILD_OPENWRT

ifeq ($(CONFIG_BIG_ENDIAN), y)
NOSTDINC_FLAGS += -DCONFIG_BIG_ENDIAN
endif
ifeq ($(CONFIG_LITTLE_ENDIAN), y)
NOSTDINC_FLAGS += -DCONFIG_LITTLE_ENDIAN
endif

PKG_MAKE_FLAGS += USER_MODULE_NAME=rtl8812au
PKG_MAKE_FLAGS += USER_DRV_NAME=rtl8812au
KERNEL_MAKE_FLAGS += CONFIG_88XXAU=m

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C "$(LINUX_DIR)" \
		$(KERNEL_MAKE_FLAGS) \
		$(PKG_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		modules
endef

$(eval $(call KernelPackage,rtl8812au-ac))
