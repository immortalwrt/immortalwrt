# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/immortalwrt/keyring.git
PKG_MIRROR_HASH:=c8600265b922462431c8f87a8cf3cd12f3e55b3c54078bb15bad9a8f41be2424
PKG_SOURCE_DATE:=2021-08-06
PKG_SOURCE_VERSION:=746fa1a0c45bd82d9bf25d5334933a2e454fdeeb

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
	# Public usign key for 18.06 release builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/1035ac73cc4e59e3 $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,openwrt-keyring))
