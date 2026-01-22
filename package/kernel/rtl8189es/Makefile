# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8189es
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://github.com/jwrdegoede/rtl8189ES_linux.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2025-09-27
PKG_SOURCE_VERSION:=0a5d04114fac3c9f48a343cb905fbb6a3f9f5df5
PKG_MIRROR_HASH:=bd06332470813b5e6919e051c53a897edcbddf97529d893ab0d97eeb74c058ac

PKG_BUILD_PARALLEL:=1

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8189es
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8189ETV SDIO WiFi
  DEPENDS:=+kmod-cfg80211 +kmod-mmc
  FILES:=$(PKG_BUILD_DIR)/rtl8189es.ko
  AUTOLOAD:=$(call AutoProbe,rtl8189es)
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
	-DBUILD_OPENWRT

define Build/Compile
	+$(KERNEL_MAKE) $(PKG_JOBS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8189ES=m \
		USER_MODULE_NAME=rtl8189es \
		modules
endef

define KernelPackage/rtl8189es/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_DATA) ./files/50_rtl-wifi $(1)/etc/uci-defaults/
endef

$(eval $(call KernelPackage,rtl8189es))
