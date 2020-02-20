#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=msgpack-c
PKG_VERSION:=3.2.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/msgpack/msgpack-c.git
PKG_SOURCE_VERSION:=8085ab8721090a447cf98bb802d1406ad7afe420
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_HASH:=464f46744a6be778626d11452c4db3c2d09461080c6db42e358e21af19d542f6

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

CMAKE_OPTIONS+= -DMSGPACK_BUILD_EXAMPLES:BOOL=OFF

define Package/msgpack-c
	SECTION:=lib
	CATEGORY:=Libraries
	URL:=https://github.com/msgpack/msgpack-c
	TITLE:=MessagePack implementation for C and C++ / msgpack.org[C/C++]
endef

define Package/libmsgpack-c
	$(call Package/msgpack-c)
	TITLE+= (Libraries)
	VARIANT:=lib
endef

define Package/msgpack-c/description
	MessagePack is an efficient binary serialization format, which lets you exchange data among multiple languages like JSON, except that it's faster and smaller. Small integers are encoded into a single byte and short strings require only one extra byte in addition to the strings themselves.
endef

define Package/libmsgpack-c/description
	$(call Package/msgpack-c/description)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include $(1)/usr/lib/pkgconfig $(1)/usr/lib/cmake
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmsgpackc.* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/msgpack.h $(1)/usr/include/msgpack.h
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/msgpack.hpp $(1)/usr/include/msgpack.hpp
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/msgpack.pc $(1)/usr/lib/pkgconfig/msgpack.pc
	$(CP) $(PKG_BUILD_DIR)/include/msgpack $(1)/usr/include/msgpack
	$(CP) $(PKG_BUILD_DIR)/CMakeFiles/Export/lib/cmake/msgpack $(1)/usr/lib/cmake/msgpack
endef

define Package/libmsgpack-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libmsgpackc.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,msgpack-c))
$(eval $(call BuildPackage,libmsgpack-c))
