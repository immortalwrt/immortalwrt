#
# Copyright (C) 2015-2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-ngrokc
PKG_VERSION:=1.3
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-ngrokc
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI support for ngrokc.
	PKGARCH:=all
	DEPENDS:=+ngrokc
endef

define Package/luci-app-ngrokc/description
	LuCI Support for ngrokc.
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -rf /tmp/luci*
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/ngrokc.lua $(1)/usr/lib/lua/luci/controller/ngrokc.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/ngrokc
	$(INSTALL_DATA) ./luasrc/model/cbi/ngrokc/*.lua $(1)/usr/lib/lua/luci/model/cbi/ngrokc/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/ngrokc
	$(INSTALL_DATA) ./luasrc/view/ngrokc/*.htm $(1)/usr/lib/lua/luci/view/ngrokc/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./root/etc/config/ngrokc $(1)/etc/config/ngrokc
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/ngrokc $(1)/etc/init.d/ngrokc
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-ngrokc $(1)/etc/uci-defaults/luci-ngrokc
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/ngrokc.po $(1)/usr/lib/lua/luci/i18n/ngrokc.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
