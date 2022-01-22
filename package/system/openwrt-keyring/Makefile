# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/immortalwrt/keyring.git
PKG_SOURCE_DATE:=2022-01-22
PKG_SOURCE_VERSION:=57304d019d9d9d124d02bf69caba707016ce9e71
PKG_MIRROR_HASH:=313e39e437415e1ba17d047d2e8d90362b8f195ec4d8f3811dca19f707f6d2e6

PKG_MAINTAINER:=John Crispin <john@phrozen.org>
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/openwrt-keyring
  SECTION:=base
  CATEGORY:=Base system
  PROVIDES:=lede-keyring
  TITLE:=ImmortalWrt Developer Keyring
  URL:=https://openwrt.org/docs/guide-user/security/signatures
endef

define Package/openwrt-keyring/description
  The keyring of with the developer using and gpg public keys.
endef

Build/Compile=

define Package/openwrt-keyring/install
	$(INSTALL_DIR) $(1)/etc/opkg/keys/
	# Public usign key for OPDE
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/66ceb3e8f7432fed $(1)/etc/opkg/keys/
	# Public usign key for 21.02 release builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/2f8b0b98e08306bf $(1)/etc/opkg/keys/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/1d072ccfc904f10a $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,openwrt-keyring))
