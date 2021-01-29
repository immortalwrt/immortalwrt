#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=parted
PKG_VERSION:=3.3
PKG_RELEASE:=2

PKG_SOURCE:=parted-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/parted
PKG_HASH:=57e2b4bd87018625c515421d4524f6e3b55175b472302056391c5f7eccb83d44

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/parted
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=parted Partition editor
  URL:=http://www.gnu.org/software/parted/index.shtml
  DEPENDS:=+libuuid +libreadline +libncurses +libblkid
endef

define Package/parted/description
	parted Partition editor
	http://www.gnu.org/software/parted/index.shtml
endef

CONFIGURE_ARGS += \
	--without-readline \
	--disable-device-mapper \
	--disable-nls

define Package/parted/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/parted $(1)/usr/sbin/parted
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/partprobe $(1)/usr/sbin/partprobe

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP)  $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,parted))
