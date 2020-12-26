#
# Copyright (C) 2016-2018 SCUT Router Term
#
# This is free software, licensed under the GNU Affero General Public License v3.
# See /COPYING for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=scutclient
PKG_VERSION:=3.1.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/scutclient/scutclient.git
PKG_MIRROR_HASH:=31e598d418956e236de287834f9553026f7853b06c5ba091fbf6f933bc305116
PKG_REV:=10d0c13cc5902925c479f1d50a68564c3129aebc

PKG_SOURCE_VERSION:=$(PKG_REV)
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_MAINTAINER:=Scutclient Project

PKG_LICENSE:=AGPL-3.0

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/scutclient
  SECTION:=net
  CATEGORY:=Network
  TITLE:=SCUT Dr.com client
  URL:=https://github.com/scutclient/scutclient
  SUBMENU:=Campus Network
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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scutclient $(1)/usr/bin
endef

$(eval $(call BuildPackage,scutclient))
