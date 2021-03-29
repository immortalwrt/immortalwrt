#
# Copyright (C) 2014-2018 OpenWrt-dist
# Copyright (C) 2014-2018 Jian Chang <aa65535@live.com>
# Copyright (C) 2021 ImmortalWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=chinadns
PKG_VERSION:=1.3.3
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/aa65535/ChinaDNS/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=74e53af32f8aa2ca7e63697385f12d89a06c486641556cfd8bc3f085d87e55ad
PKG_BUILD_DIR:=$(BUILD_DIR)/ChinaDNS-$(PKG_VERSION)

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/chinadns
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Protect yourself against DNS poisoning in China.
  URL:=https://github.com/shadowsocks/ChinaDNS
endef

define Package/chinadns/conffiles
/usr/share/chinadns/chnroute.txt
/usr/share/chinadns/iplist.txt
endef

define Package/chinadns/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/chinadns $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/chinadns
	$(CP) $(PKG_INSTALL_DIR)/usr/share/chnroute.txt $(1)/usr/share/chinadns/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/iplist.txt $(1)/usr/share/chinadns/
endef

$(eval $(call BuildPackage,chinadns))
