#
# Copyright (C) 2018 Hao Dong <halbertdong@gmail.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-speederv2
PKG_VERSION:=1.1.0
PKG_RELEASE:=3

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Hao Dong <halbertdong@gmail.com>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for speederv2-tunnel
	PKGARCH:=all
	DEPENDS:=+speederv2-tunnel
endef

define Package/$(PKG_NAME)/description
	LuCI Support for speederv2-tunnel.
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
	( . /etc/uci-defaults/luci-speederv2 ) && rm -f /etc/uci-defaults/luci-speederv2
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/speederv2
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/speederv2.*.lmo $(1)/usr/lib/lua/luci/i18n/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./files/luci/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/speederv2
	$(INSTALL_DATA) ./files/luci/model/cbi/speederv2/*.lua $(1)/usr/lib/lua/luci/model/cbi/speederv2/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/speederv2
	$(INSTALL_DATA) ./files/luci/view/speederv2/*.htm $(1)/usr/lib/lua/luci/view/speederv2/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/root/etc/config/speederv2 $(1)/etc/config/speederv2
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/root/etc/init.d/speederv2 $(1)/etc/init.d/speederv2
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/root/etc/uci-defaults/luci-speederv2 $(1)/etc/uci-defaults/luci-speederv2
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
