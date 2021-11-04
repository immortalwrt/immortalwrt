#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libericstools
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_MAINTAINER:=Eric Bishop <eric@gargoyle-router.com>

include $(INCLUDE_DIR)/package.mk

define Package/libericstools
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Erics Tools
  URL:=http://www.gargoyle-router.com
endef

define Package/libericstools/description
  A bunch of routines/utilities written by Eric Bishop, a library
  primarily used in Gargoyle Web Interface for OpenWrt.
endef

define Build/Configure
endef

define Build/InstallDev
	mkdir -p $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/*.h $(STAGING_DIR)/usr/include/

	mkdir -p $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.so* $(STAGING_DIR)/usr/lib/
endef

define Package/libericstools/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libericstools))
