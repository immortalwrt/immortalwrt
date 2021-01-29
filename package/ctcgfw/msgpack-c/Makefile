#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=msgpack-c
PKG_VERSION:=3.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/msgpack/msgpack-c/tar.gz/cpp-$(PKG_VERSION)?
PKG_HASH:=754c3ace499a63e45b77ef4bcab4ee602c2c414f58403bce826b76ffc2f77d0b
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-cpp-$(PKG_VERSION)

PKG_LICENSE:=BSL-1.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DMSGPACK_BUILD_EXAMPLES:BOOL=OFF

define Package/libmsgpack-c
	SECTION:=lib
	CATEGORY:=Libraries
	URL:=https://github.com/msgpack/msgpack-c
	TITLE:=MessagePack implementation for C and C++ / msgpack.org[C/C++]
endef

define Package/libmsgpack-c/description
	MessagePack is an efficient binary serialization format, which lets you exchange data among multiple languages like JSON, except that it's faster and smaller. Small integers are encoded into a single byte and short strings require only one extra byte in addition to the strings themselves.
endef

define Package/libmsgpack-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmsgpackc.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmsgpack-c))
