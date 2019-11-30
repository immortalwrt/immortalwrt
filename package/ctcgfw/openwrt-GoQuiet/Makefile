#
# Copyright (C) 2018 Andrea Muratori <andrea.m4niac@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=GoQuiet
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/cbeuw/GoQuiet.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_RELEASE)
PKG_SOURCE_VERSION:=2f4a456b85d9ede8399546518018c11966511b5a
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Andrea Muratori <andrea.m4niac@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)/$(PKG_SOURCE_SUBDIR)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/GoQuiet/Default
	SECTION:=net
	CATEGORY:=Network
	TITLE:=GoQuiet
	URL:=https://github.com/cbeuw/GoQuiet/
endef

Package/gq-client = $(call Package/GoQuiet/Default)
Package/gq-server = $(call Package/GoQuiet/Default)

define Package/gq-client/description
A shadowsocks plugin that obfuscates the traffic as normal HTTPS traffic and disguises the proxy server as a normal webserver.

Package/gq-server/description = $(Package/simple-obfs/description)

CONFIGURE_ARGS += --disable-ssp --disable-documentation --disable-assert

define Package/gq-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/GoQuiet-client $(1)/usr/bin/gq-client
endef

define Package/gq-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/GoQuiet-server $(1)/usr/bin/gq-server
endef

$(eval $(call BuildPackage,gq-client))
$(eval $(call BuildPackage,gq-server))
