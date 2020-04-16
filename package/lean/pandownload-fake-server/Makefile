#
# Copyright (C) 2016-2017 GitHub 
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.

include $(TOPDIR)/rules.mk

PKG_NAME:=pandownload-fake-server
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

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

define Build/Configure
endef

define Build/Compile
endef

define Package/pandownload-fake-server/install
	$(INSTALL_DIR) $(1)/www
	$(CP) ./files/* $(1)/www/
endef

$(eval $(call BuildPackage,pandownload-fake-server))
