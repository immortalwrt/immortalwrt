#
# Copyright (C) 2011-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=polarssl
PKG_VERSION:=1.3.22
PKG_RELEASE:=1

PKG_SOURCE_NAME:=mbedtls
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION)-gpl.tgz
PKG_SOURCE_URL:=https://tls.mbed.org/download/
PKG_HASH:=ded041aa4acf9a3e4d0c85bf334b0860135da996e75afdcc3abf22f403d14457

PKG_LICENSE:=GPL-2.0-or-later
PKG_LICENSE_FILE:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/libpolarssl
  SECTION:=libs
  CATEGORY:=Libraries
  SUBMENU:=SSL
  TITLE:=Embedded SSL
  URL:=http://polarssl.org/
  ABI_VERSION:=$(PKG_VERSION)-$(PKG_RELEASE)
endef

define Package/libpolarssl/description
  The aim of the PolarSSL project is to provide a quality, open-source
  cryptographic library written in C and targeted at embedded systems.

  This package contains the PolarSSL library.
endef

CMAKE_OPTIONS += \
	-DCMAKE_BUILD_TYPE:String="Release" \
	-DUSE_SHARED_MBEDTLS_LIBRARY:Bool=ON \
	-DENABLE_TESTING:Bool=OFF \
	-DENABLE_PROGRAMS:Bool=OFF \

define Build/InstallDev
	$(Build/InstallDev/cmake)
	$(LN) libmbedtls.so.$(PKG_VERSION) $(1)/usr/lib/libpolarssl.so
endef

define Package/libpolarssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libmbedtls.so.* $(1)/usr/lib/
	$(LN) libmbedtls.so.$(PKG_VERSION) $(1)/usr/lib/libpolarssl.so
endef

$(eval $(call BuildPackage,libpolarssl))
