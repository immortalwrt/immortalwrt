#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=jpcre2
PKG_VERSION:=10.32.01

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/jpcre2/jpcre2/tar.gz/$(PKG_VERSION)?
PKG_HASH:=e8c69a6a6f77ed8d5c35dcdfa767b155f5230a83f55d35cab837b6f0a1266ca8
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=BSD 3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

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
