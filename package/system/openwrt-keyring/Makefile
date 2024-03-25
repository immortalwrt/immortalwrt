# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/immortalwrt/keyring.git
PKG_SOURCE_DATE:=2021-11-13
PKG_SOURCE_VERSION:=07d34f5cd9cc27aeaaa0c04358370ca1e9fbb0fc
PKG_MIRROR_HASH:=2fa6812c1ed5169a8dea7416bc19270b1656f33bd94506a69609a40f7713749d

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
	# Public usign key for unattended snapshot builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/b5043e70f9a75cde $(1)/etc/opkg/keys/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/8ead5420c4005b0d $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,openwrt-keyring))
