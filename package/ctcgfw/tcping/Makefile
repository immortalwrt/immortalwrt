#
# Copyright (C) 2014-2018 OpenWrt-dist
# Copyright (C) 2019 [CTCGFW]Project-OpenWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tcping-simple
PKG_VERSION:=1.3.5
PKG_RELEASE:=1

PKG_SOURCE_URL:=https://raw.githubusercontent.com/MushrooM93/tcping/b9ee61a5c1d05f228aa70027331556f0c27dd4c2/
PKG_SOURCE:=tcping.c
PKG_HASH:=3459b9f15621920dc632b5871da96c62c8a77ad15ee7bfdba4adee29f4eed089
PKG_BUILD_DIR:=$(BUILD_DIR)/tcping-$(PKG_VERSION)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=MushrooM93 <799163169@qq.com>

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=tcping support ipv4/ipv6 dual-stack in linux, the operation is similar to ping but it does not use the ICMP protocol.
	URL:=https://github.com/MushrooM93/tcping
endef

define Package/$(PKG_NAME)/description
tcping support ipv4/ipv6 dual-stack in linux, the operation is similar to ping but it does not use the ICMP protocol.
endef

define Build/Prepare
	cp -f $(DL_DIR)/$(PKG_SOURCE) $(PKG_BUILD_DIR)/$(PKG_SOURCE)
endef

define Build/Compile
	$(TARGET_CROSS)gcc $(PKG_BUILD_DIR)/$(PKG_SOURCE) -o $(PKG_BUILD_DIR)/tcping
endef

define Build/Install
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tcping-simple $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
