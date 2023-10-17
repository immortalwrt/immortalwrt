# Copyright (C) 2016 LEDE project

include $(TOPDIR)/rules.mk

PKG_NAME:=openwrt-keyring
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/immortalwrt/keyring.git
PKG_SOURCE_DATE:=2023-05-22
PKG_SOURCE_VERSION:=455b399d19bc2fb9ae08e89fee3d295613626969
PKG_MIRROR_HASH:=c94813e19655c53ea903315199bf2fea57e2fd1506b7f211ccaa786b11ae2f81

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
	# Public usign key for unattended snapshot builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/b5043e70f9a75cde $(1)/etc/opkg/keys/
	# Public usign key for 23.05 release builds
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/usign/682970064b87a917 $(1)/etc/opkg/keys/
endef

$(eval $(call BuildPackage,openwrt-keyring))
