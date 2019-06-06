#
# Copyright (C) 2016-2017 WindTech <admin@windtech.top>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-filebrowser-mipsle
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=WindTech <admin@windtech.top>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for FileBrowser FOR MIPSLE ONLY
	PKGARCH:=all
#	DEPENDS:=+filebrowser
	DEPENDS:=+wget +openssl-util
endef

define Package/$(PKG_NAME)/description
	LuCI Support for FileBrowser FOR MIPSLE ONLY.
endef

define Build/Prepare
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-filebrowser ) && rm -f /etc/uci-defaults/luci-filebrowser
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/filebrowser
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) ./files/luci/i18n/filebrowser.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/filebrowser.zh-cn.lmo
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/filebrowser.lua $(1)/usr/lib/lua/luci/controller/filebrowser.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/filebrowser
	$(INSTALL_DATA) ./files/luci/model/cbi/filebrowser.lua $(1)/usr/lib/lua/luci/model/cbi/filebrowser.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/filebrowser $(1)/etc/config/filebrowser
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/filebrowser $(1)/etc/init.d/filebrowser
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/luci-filebrowser $(1)/etc/uci-defaults/luci-filebrowser
endef

$(eval $(call BuildPackage,$(PKG_NAME)))