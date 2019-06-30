#
# Copyright (C) 2016-2017 WindTech <admin@windtech.top>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-opendoor
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CTCGFW

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:= FBI! Open the door!
	PKGARCH:=all
endef

define Package/$(PKG_NAME)/description
	FBI! Open the door!
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
endef

define Package/$(PKG_NAME)/conffiles
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./google_fu_mode $(1)/etc/config/google_fu_mode
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
