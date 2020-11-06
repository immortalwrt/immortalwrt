#
# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8192du
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lwfinger/rtl8192du
PKG_SOURCE_DATE:=2019-06-01
PKG_SOURCE_VERSION:=54c95aaa23fe332afaa1ed4aacbb0db43aa3c7dd
PKG_MIRROR_HASH:=d7cee3b7e8601c8e850570401628128f595faa4ad2895237708ba0e58bee3020

PKG_BUILD_PARALLEL:=1
PKG_MAINTAINER:=Larry Finger <Larry.Finger@lwfinger.net>

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192du
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8192DU support
  DEPENDS:=+kmod-cfg80211 +kmod-usb-core +@DRIVER_11N_SUPPORT
  FILES:= $(PKG_BUILD_DIR)/8192du.ko
  AUTOLOAD:=$(call AutoProbe,8192du)
endef

define KernelPackage/rtl8192du/description
  Kernel modules for the Realtek 8192DU USB 802.11n wireless USB adapters
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

NOSTDINC_FLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT -DCONFIG_CONCURRENT_MODE -DBUILD_OPENWRT

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(LINUX_DIR) \
		$(KERNEL_MAKE_FLAGS) \
		M="$(PKG_BUILD_DIR)" \
		NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
		CONFIG_RTL8192DU=m \
		modules
endef

define KernelPackage/rtl8192du/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtlwifi
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw.bin $(1)/lib/firmware/rtlwifi/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw_wol.bin $(1)/lib/firmware/rtlwifi/
endef

$(eval $(call KernelPackage,rtl8192du))
