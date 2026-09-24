# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (c) 2026 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=uwe5622
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/armbian/uwe5622.git
PKG_SOURCE_DATE:=2026-09-13
PKG_SOURCE_VERSION:=cc2835a3f935d5297e03cdce464c1785381a7b4d
PKG_MIRROR_HASH:=17a817f18895db3552515e60603b709d23d2949848950b4b0513b7e5ed6a233c

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/uwe5622
  SUBMENU:=Wireless Drivers
  TITLE:=AW859A / UWE5622 Linux driver
  DEPENDS:=+uwe5622-firmware +kmod-mmc +kmod-cfg80211 +@DRIVER_11AC_SUPPORT +@DRIVER_11N_SUPPORT @TARGET_sunxi
  FILES:= \
	$(PKG_BUILD_DIR)/unisocwcn/uwe5622_bsp_sdio.ko \
	$(PKG_BUILD_DIR)/unisocwifi/sprdwl_ng.ko
  AUTOLOAD:=$(call AutoProbe,sprdwl_ng uwe5622_bsp_sdio)
endef

define KernelPackage/uwe5622/Description
  Linux kernel drivers for the Unisoc/Spreadtrum uwe5622 wireless
  connectivity chipset.
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
	-include backport/backport.h \
	-DBUILD_OPENWRT

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/unisocwcn \
		CONFIG_AW_WIFI_DEVICE_UWE5622=y \
		CFLAGS_MODULE="-DCONFIG_AW_BIND_VERIFY=1" \
		UNISOC_FW_PATH_CONFIG=/lib/firmware/uwe5622/ \
		modules
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M=$(PKG_BUILD_DIR)/unisocwifi \
		CONFIG_WLAN_UWE5622=m \
		UNISOC_WIFI_CUS_CONFIG=/lib/firmware/uwe5622 \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		KBUILD_EXTRA_SYMBOLS="$(wildcard $(PKG_SYMVERS_DIR)/*.symvers) $(PKG_BUILD_DIR)/unisocwcn/Module.symvers" \
		modules
endef

$(eval $(call KernelPackage,uwe5622))
