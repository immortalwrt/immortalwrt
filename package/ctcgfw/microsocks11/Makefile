#
# Copyright (C) 2021 ImmortalWrt
# (https://immortalwrt.org)
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=microsocks11
PKG_VERSION:=2.3.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/EvanMcBroom/microsocks11/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=7b2f7c1fca9ba6cb135c9ec3f2f7304b6ccd790bc15318619f515b3b2e5e148e

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

PKG_BUILD_DEPENDS:=cxxopts
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/microsocks11
  SECTION:=net
  CATEGORY:=Network
  URL:=https://github.com/EvanMcBroom/microsocks11
  TITLE:=A cross-platform SOCKS5 server based on the microsocks project
  DEPENDS:=+libc +libstdcpp +libpthread
  CONFLICTS:=microsocks
endef

define Package/libmicrosocks11
  SECTION:=lib
  CATEGORY:=Libraries
  URL:=https://github.com/EvanMcBroom/microsocks11
  TITLE:=A cross-platform SOCKS5 library based on the microsocks project
  DEPENDS:=+libc +libstdcpp +libpthread
endef

define Package/microsocks11/description
  MicroSocks11 is a SOCKS5 library and server based on the microsocks project that uses
  CMake and C++11 for cross-platform support. MicroSocks11 supports IPv4, IPv6, and DNS.
endef

Package/libmicrosocks11/description = $(Package/microsocks11/description)

define Build/Install
	true
endef

define Package/microsocks11/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/microsocks $(1)/usr/bin/microsocks
endef

define Package/libmicrosocks11/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/libsocks5.a $(1)/usr/lib/libsocks5.a
endef

$(eval $(call BuildPackage,microsocks11))
$(eval $(call BuildPackage,libmicrosocks11))
