#
# Copyright (C) 2016-2017 WindTech <admin@windtech.top>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-baidupcs
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
	TITLE:=LuCI Support for BaiduPCS-Go FOR MIPSLE ONLY
	PKGARCH:=all
#	DEPENDS:=+baidupcs-go
	DEPENDS:=+wget +openssl-util
endef

define Package/$(PKG_NAME)/description
	LuCI Support for BaiduPCS-Go FOR MIPSLE ONLY.
endef

define Build/Prepare
	$(foreach po,$(wildcard ${CURDIR}/files/luci/i18n/*.po), \
		po2lmo $(po) $(PKG_BUILD_DIR)/$(patsubst %.po,%.lmo,$(notdir $(po)));)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-baidupcs ) && rm -f /etc/uci-defaults/luci-baidupcs
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/baidupcs
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) ./files/luci/i18n/baidupcs.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/baidupcs.zh-cn.lmo
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/baidupcs.lua $(1)/usr/lib/lua/luci/controller/baidupcs.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/baidupcs
	$(INSTALL_DATA) ./files/luci/model/cbi/baidupcs.lua $(1)/usr/lib/lua/luci/model/cbi/baidupcs.lua
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/baidupcs $(1)/etc/config/baidupcs
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/etc/init.d/baidupcs $(1)/etc/init.d/baidupcs
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/luci-baidupcs $(1)/etc/uci-defaults/luci-baidupcs
endef

$(eval $(call BuildPackage,$(PKG_NAME)))