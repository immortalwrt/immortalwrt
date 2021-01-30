#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=srelay
PKG_VERSION:=0.4.8p3
PKG_RELEASE:=9

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/socks-relay
PKG_HASH:=efa38cb3e9e745a05ccb4b59fcf5d041184f15dbea8eb80c1b0ce809bb00c924

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/srelay
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Web Servers/Proxies
  TITLE:=A socks 4/5 proxy server
  URL:=http://www.c-wind.com/srelay/
  DEPENDS:=+libpthread
endef

define Package/srelay/conffiles
/etc/srelay.conf
endef

CONFIGURE_ARGS += \
	--enable-thread \
	--with-libwrap=no \

CONFIGURE_VARS += \
	CPPFLAGS="-DLINUX $$$$CPPFLAGS" \

define Package/srelay/install	
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin/
endef

$(eval $(call BuildPackage,srelay))
