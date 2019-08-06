#
# Copyright (C) 2019 [CTCGFW]Project-OpenWRT
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic-nondanee
PKG_VERSION:=latest
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project-OpenWRT

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Revive unavailable songs for Netease Cloud Music on OpenWRT
	URL:=https://github.com/nondanee/UnblockNeteaseMusic
	DEPENDS:=+node
endef

define Package/$(PKG_NAME)/description
Revive unavailable songs for Netease Cloud Music on OpenWRT
endef

define Build/Prepare
	rm -f $(DL_DIR)/$(PKG_NAME).tar.gz
	wget https://github.com/nondanee/UnblockNeteaseMusic/archive/master.tar.gz -O $(DL_DIR)/$(PKG_NAME).tar.gz
	tar -xzf $(DL_DIR)/$(PKG_NAME).tar.gz -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share/unblockneteasemusic
	cp -fpR $(PKG_BUILD_DIR)/UnblockNeteaseMusic-master/* $(1)/usr/share/unblockneteasemusic/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
