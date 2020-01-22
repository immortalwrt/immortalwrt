# Copyright (C) 2006-2018 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8192du
PKG_VERSION:=2019-06-01
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lwfinger/rtl8192du
PKG_SOURCE_VERSION:=54c95aaa23fe332afaa1ed4aacbb0db43aa3c7dd
PKG_MIRROR_HASH:=9c21ca7b9115de8e711b38c3005fb22af5a9f392f801ac1f2383d82df789e81a
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz

PKG_MAINTAINER:=Javier Marcet <javier@keezel.co>
PKG_LICENSE:=GPLv2

STAMP_CONFIGURED_DEPENDS := $(STAGING_DIR)/usr/include/mac80211-backport/backport/autoconf.h

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

NOSTDINC_FLAGS = \
		-I$(PKG_BUILD_DIR)/include \
		-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
		-I$(STAGING_DIR)/usr/include/mac80211-backport \
		-I$(STAGING_DIR)/usr/include/mac80211/uapi \
		-I$(STAGING_DIR)/usr/include/mac80211 \
		-include backport/autoconf.h \
		-include backport/backport.h

define KernelPackage/rtl8192du
  SUBMENU:=Wireless Drivers
  TITLE:=Realtek RTL8192DU wireless USB 802.11n driver
  DEPENDS:=@USB_SUPPORT +kmod-cfg80211 +kmod-mac80211 +kmod-usb-core +wireless-tools
  FILES:= $(PKG_BUILD_DIR)/8192du.$(LINUX_KMOD_SUFFIX)
# AUTOLOAD:=$(call AutoProbe,8192du)
endef

define KernelPackage/rtl8192du/description
  Kernel modules for the Realtek 8192DU USB 802.11n wireless USB adapters
endef

define Build/Compile
		+$(MAKE) $(PKG_JOBS) -C $(LINUX_DIR) \
				$(KERNEL_MAKE_FLAGS) \
				SUBDIRS="$(PKG_BUILD_DIR)" \
				CONFIG_RTL8192DU=m \
				NOSTDINC_FLAGS="$(NOSTDINC_FLAGS)" \
				M="$(PKG_BUILD_DIR)" \
				src="$(PKG_BUILD_DIR)" \
				TopDIR="$(PKG_BUILD_DIR)" \
				modules
endef

define Build/Configure
endef

define KernelPackage/rtl8192du/install
		$(INSTALL_DIR) $(1)/lib/firmware/rtlwifi
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw.bin $(1)/lib/firmware/rtlwifi/
		$(INSTALL_DATA) $(PKG_BUILD_DIR)/rtl8192dufw_wol.bin $(1)/lib/firmware/rtlwifi/
endef

$(eval $(call KernelPackage,rtl8192du))
