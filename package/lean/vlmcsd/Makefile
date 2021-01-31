#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vlmcsd
PKG_VERSION:=svn1113
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/Wind4/vlmcsd/tar.gz/$(PKG_VERSION)?
PKG_HASH:=62f55c48f5de1249c2348ab6b96dabbe7e38899230954b0c8774efb01d9c42cc

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=fuyumi <280604399@qq.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/vlmcsd
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A KMS Emulator in C
	URL:=https://github.com/Wind4/vlmcsd
	DEPENDS:=+libpthread
endef

define Package/vlmcsd/description
	KMS Emulator in C (currently runs on Linux including Android, FreeBSD,
	Solaris, Minix, Mac OS, iOS, Windows with or without Cygwin)
endef

define Package/vlmcsd/install
	$(INSTALL_DIR) $(1)/usr/bin
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vlmcs $(1)/usr/bin/vlmcs
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/vlmcsd $(1)/usr/bin/vlmcsd

	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/vlmcsd.ini $(1)/etc/vlmcsd.ini
	#$(INSTALL_DIR) $(1)/etc/init.d
	#$(INSTALL_BIN) ./files/vlmcsd.init $(1)/etc/init.d/vlmcsd
endef

$(eval $(call BuildPackage,vlmcsd))
