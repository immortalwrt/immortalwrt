#
# Copyright (C) 2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-lib-fs
PKG_VERSION:=1.0
PKG_RELEASE:=2

include $(INCLUDE_DIR)/package.mk

define Package/luci-lib-fs
  SECTION:=lang
  CATEGORY:=Languages
  SUBMENU:=Lua
  TITLE:=luci-lib-fs
  URL:=https://github.com/lbthomsen/openwrt-luci
  DEPENDS:=+luci +luci-lib-nixio
  PKGARCH:=all
endef

define Build/Compile
	true
endef

define Package/luci-lib-fs/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	$(CP) ./files/fs.lua $(1)/usr/lib/lua/luci/fs.lua
endef

$(eval $(call BuildPackage,luci-lib-fs))
