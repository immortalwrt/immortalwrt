#
# Copyright (C) 2017-2018 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ssocks
PKG_VERSION:=0.0.14
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/tostercx/ssocks.git
PKG_SOURCE_VERSION:=8d9a5d2f89e507c9ffebd838b3c7430f07a0f36c
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=[CTCGFW]Project OpenWrt

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=sSocks
	URL:=https://github.com/david378/ssocks
endef

define Package/$(PKG_NAME)/description
sSocks is a package which contains: a socks5 server implements RFC 1928 (SOCKS V5) and RFC 1929 (Authentication for SOCKS V5), a reverse socks server and client, a netcat like tool and a socks5 relay.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/ssocksd $(1)/usr/bin/ssocks
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
