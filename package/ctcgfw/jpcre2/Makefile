#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=jpcre2
PKG_VERSION:=10.31.03

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/jpcre2/jpcre2.git
PKG_SOURCE_VERSION:=709ec0e866c3ae3ac61e5683b7f9b3b6feaac3f6
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_HASH:=22deb97515e48adf80383ca691229b83d13d8850f6e8b98710426b24955ae5de

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=pcre2

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=lib
	CATEGORY:=Libraries
	TITLE:=C++ wrapper for PCRE2 Library
	URL:=https://github.com/jpcre2/jpcre2
endef

define Package/$(PKG_NAME)/description
	This provides some C++ wrapper classes/functions to perform regex operations such as regex match and regex replace.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/jpcre2.hpp $(1)/usr/include/jpcre2.hpp
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
