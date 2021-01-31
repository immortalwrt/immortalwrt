#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vsftpd-alt
PKG_VERSION:=3.0.3
PKG_RELEASE:=8

PKG_SOURCE_NAME:=$(firstword $(subst -, ,$(PKG_NAME)))
PKG_SOURCE:=$(PKG_SOURCE_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://security.appspot.com/downloads/
PKG_HASH:=9d4d2bf6e6e2884852ba4e69e157a2cecd68c5a7635d66a3a8cf8d898c955ef7

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILE:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vsftpd-alt
  SUBMENU:=File Transfer
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A fast and secure FTP server (with aio support)
  URL:=http://vsftpd.beasts.org/
  MAINTAINER:=Cezary Jackiewicz <cezary@eko.one.pl>
  DEPENDS=+libuci 
endef

TARGET_CFLAGS += -D_GNU_SOURCE -include fcntl.h

define Build/Configure
	$(call Build/Configure/Default)
ifneq ($(CONFIG_USE_MUSL)$(CONFIG_USE_GLIBC),)
	$(SED) 's/-lcrypt -lnsl/-lcrypt/' $(PKG_BUILD_DIR)/Makefile
endif
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		vsftpd
endef

define Package/vsftpd-alt/conffiles
#/etc/vsftpd.conf
/etc/config/vsftpd
endef

define Package/vsftpd-alt/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/vsftpd $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/vsftpd_wrapper $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/vsftpd_prepare $(1)/usr/sbin/

	#$(INSTALL_DIR) $(1)/etc
	#$(INSTALL_CONF) ./files/vsftpd.conf $(1)/etc/vsftpd.conf

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/vsftpd.uci $(1)/etc/config/vsftpd
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/vsftpd-uci.init $(1)/etc/init.d/vsftpd
endef

$(eval $(call BuildPackage,vsftpd-alt))
