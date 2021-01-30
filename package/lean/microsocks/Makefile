#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=microsocks
PKG_VERSION=1.0.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/rofl0r/microsocks.git
PKG_SOURCE_DATE:=2020-10-25
PKG_SOURCE_VERSION:=31557857ccce5e4fdd2cfdae7ab640d589aa2b41
PKG_MIRROR_HASH:=fd914984fa8314d73630a31c2daad8d34de85597953669691e0c69d52f6a6010

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=lean

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/microsocks
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	TITLE:=tiny, portable SOCKS5 server with very moderate resource usage
	URL:=https://github.com/rofl0r/microsocks
	DEPENDS:=+libpthread
endef

define Package/microsocks/description
	A SOCKS5 service that you can run on your remote boxes to tunnel connections
	through them, if for some reason SSH doesn't cut it for you.
endef

define Package/microsocks/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/microsocks $(1)/usr/bin/microsocks
endef

$(eval $(call BuildPackage,microsocks))
