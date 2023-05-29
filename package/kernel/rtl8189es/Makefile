# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2021-2022 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8189es
PKG_RELEASE:=3

PKG_SOURCE_URL:=https://github.com/jwrdegoede/rtl8189ES_linux.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_DATE:=2022-10-30
PKG_SOURCE_VERSION:=e58bd86c9d9408c648b1246a0dd76b16856ec172
PKG_MIRROR_HASH:=1e4acfd7d1f1f2490b6778068f97e7a63b81575dffd98c93e84cecb287e2c7ad

PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>
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
