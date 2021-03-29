#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fcgiwrap
PKG_VERSION:=1.0
PKG_RELEASE:=$(AUTORELEASE)

PKG_MAINTAINER:=Tobias Waldvogel <tobias.waldvogel@gmail.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/fcgiwrap
  SECTION:=net
  CATEGORY:=Network
  TITLE:=FCGI wrapper for CGI
  DEPENDS:=+libpcre +spawn-fcgi +fcgi
endef

define Build/Compile
	$(TARGET_CC) \
		-I$(STAGING_DIR)/usr/include $(TARGET_CFLAGS) -Wall -Werror \
		-L$(STAGING_DIR)/usr/lib -lfcgi \
		-o$(PKG_BUILD_DIR)/fcgiwrap $(PKG_BUILD_DIR)/fcgiwrap.c
endef

define Package/fcgiwrap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fcgiwrap $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fcgiwrap.init $(1)/etc/init.d/fcgiwrap
endef

$(eval $(call BuildPackage,fcgiwrap))
