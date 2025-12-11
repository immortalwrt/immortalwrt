# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/immortalwrt/keyring.git
PKG_SOURCE_DATE:=2025-12-11
PKG_SOURCE_VERSION:=14e1f88b521f6466dfcc06b055646cfa25e8e02f
PKG_MIRROR_HASH:=1928719f41b4aa2709829618e54a1dbd1322cdccf6bc56213354bf9c50017995

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

ifneq ($(CONFIG_USE_APK),)
define Package/openwrt-keyring/install
	$(INSTALL_DIR) $(1)/etc/apk/keys/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/apk/openwrt-25.12.pem $(1)/etc/apk/keys/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/apk/immortalwrt-25.12.pem $(1)/etc/apk/keys/
endef
else
define Package/openwrt-keyring/install
	$(INSTALL_DIR) $(1)/etc/opkg/keys/
	# Public usign key for 25.12 release builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/467a35c14d7fbc5c$(1)/etc/opkg/keys/
endef
endif

$(eval $(call BuildPackage,openwrt-keyring))
