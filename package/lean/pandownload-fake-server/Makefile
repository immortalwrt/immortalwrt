#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pandownload-fake-server
PKG_VERSION:=1
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/pandownload-fake-server
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Download Manager
  TITLE:=pandownload-fake-server
  PKGARCH:=all
endef

define Package/pandownload-fake-server/description
  Fake server for Pandownload.
endef

define Build/Compile
	true
endef

define Package/pandownload-fake-server/install
	$(INSTALL_DIR) $(1)/www
	$(CP) ./files/* $(1)/www/
endef

$(eval $(call BuildPackage,pandownload-fake-server))
