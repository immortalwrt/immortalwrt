#
# Copyright (C) 2016-2018 SCUT Router Term
#
# This is free software, licensed under the GNU Affero General Public License v3.
# See /COPYING for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=scutclient
PKG_BASE_VERSION:=3.1.2
PKG_RELEASE:=$(AUTORELESE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/scutclient/scutclient.git
PKG_SOURCE_DATE:=2020-11-29
PKG_SOURCE_VERSION:=36128216c63f8954371e64d78ff30bca5e73a882
PKG_MIRROR_HASH:=16be6312e7ca2e1b0201c2d2f7e01dbb6173fb9b13c93707ff272f992d4b3bff

PKG_VERSION:=$(PKG_BASE_VERSION)-$(PKG_SOURCE_DATE)-$(call version_abbrev,$(PKG_SOURCE_VERSION))

PKG_MAINTAINER:=Scutclient Project
PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILE:=COPYING

PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/scutclient
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Campus Network
  TITLE:=SCUT Dr.com client
  URL:=https://github.com/scutclient/scutclient
endef

define Package/scutclient/description
  Support SCUT private authentication protocol.
endef

define Package/scutclient/conffiles
/etc/config/scutclient
endef

define Package/scutclient/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openwrt/files/scutclient.config $(1)/etc/config/scutclient
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.init $(1)/etc/init.d/scutclient
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openwrt/files/scutclient.hotplug $(1)/etc/hotplug.d/iface/99-scutclient
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/scutclient $(1)/usr/bin
endef

$(eval $(call BuildPackage,scutclient))
