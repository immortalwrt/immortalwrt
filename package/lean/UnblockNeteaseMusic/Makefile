#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic
PKG_VERSION:=0.25.3
PKG_RELEASE:=6

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/nondanee/UnblockNeteaseMusic.git
PKG_SOURCE_DATE:=2020-12-22
PKG_SOURCE_VERSION:=b8d5f4b4fe59cef9c021b99f3cf2606d99fa4790
PKG_MIRROR_HASH:=bd7fa102415ae8dbcd22829fce5ad0cd358a0869547c30f10271389e6c698317

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/UnblockNeteaseMusic
	SECTION:=multimedia
	CATEGORY:=Multimedia
	TITLE:=Revive Netease Cloud Music (NodeJS)
	URL:=https://github.com/nondanee/UnblockNeteaseMusic
	DEPENDS:=+node +libopenssl
	PKGARCH:=all
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	xzcat $(DL_DIR)/$(PKG_SOURCE) | $(TAR) -C $(PKG_BUILD_DIR) $(TAR_OPTIONS)
endef

define Build/Configure
	echo -e $(PKG_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/core_ver
	echo -e $(PKG_SOURCE_VERSION) > $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/local_ver
endef

define Build/Compile
	true
endef

define Package/UnblockNeteaseMusic/install
	$(INSTALL_DIR) $(1)/usr/share/UnblockNeteaseMusic
	$(CP) $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/* $(1)/usr/share/UnblockNeteaseMusic
endef

$(eval $(call BuildPackage,UnblockNeteaseMusic))
