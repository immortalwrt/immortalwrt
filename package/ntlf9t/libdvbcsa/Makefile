#
# Copyright (C) 2014 nanpuyue <nanpuyue@gmail.com>
# Copyright (C) 2017 Tomasz Maciej Nowak <tomek_n@o2.pl>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=libdvbcsa
PKG_BASE_VERSION:=1.1.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/glenvt18/libdvbcsa.git
PKG_SOURCE_DATE:=2018-01-28
PKG_SOURCE_VERSION:=2a1e61e569a621c55c2426f235f42c2398b7f18f
PKG_MIRROR_HASH:=125c5d42fd1c73d3046ab9ee52d11d4c6495a2df45daa3eaf7bc5412dd91d586

PKG_VERSION:=$(PKG_BASE_VERSION)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILE:=COPYING
PKG_MAINTAINER:=Tomasz Maciej Nowak <tomek_n@o2.pl>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/libdvbcsa
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=DVB Common Scrambling Algorithm library
  URL:=https://www.videolan.org/developers/libdvbcsa.html
endef

define Package/libdvbcsa/description
 libdvbcsa is a free implementation of the DVB Common Scrambling
 Algorithm - DVB/CSA - with encryption and decryption capabilities
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); ./bootstrap)
	$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libdvbcsa/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdvbcsa.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libdvbcsa))
