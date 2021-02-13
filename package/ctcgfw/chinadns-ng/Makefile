#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chinadns-ng
PKG_VERSION:=1.0-beta.24
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/zfl9/chinadns-ng.git
PKG_SOURCE_DATE:=2021-02-11
PKG_SOURCE_VERSION:=df3bb7c6310df9b3c0f874b7a0c38ea5ce0996cb
PKG_MIRROR_HASH:=4b0a159e1ecd81392a67b31528b0430afa991bfceef5dd60d2d843edfccb6b12

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=pexcn <i@pexcn.me>

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/chinadns-ng
	SECTION:=net
	CATEGORY:=Network
	TITLE:=ChinaDNS next generation, refactoring with epoll and ipset.
	URL:=https://github.com/zfl9/chinadns-ng
	DEPENDS:=+ipset
endef

define Package/chinadns-ng/description
	ChinaDNS next generation, refactoring with epoll and ipset.
endef

define Package/chinadns-ng/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/chinadns-ng $(1)/usr/bin
endef

$(eval $(call BuildPackage,chinadns-ng))
