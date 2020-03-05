#
# Copyright (C) 2017 Ian Li <OpenSource@ianli.xyz>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-chinadns
PKG_VERSION:=2.0.1
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Ian Li <OpenSource@ianli.xyz>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-chinadns
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for ChinaDNS
	PKGARCH:=all
	DEPENDS:=+ChinaDNS +curl
endef

define Package/luci-app-chinadns/description
	LuCI Support for ChinaDNS.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-chinadns/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ -f /etc/uci-defaults/luci-chinadns ]; then
		( . /etc/uci-defaults/luci-chinadns ) && \
		rm -f /etc/uci-defaults/luci-chinadns
	fi
	rm -rf /tmp/luci-indexcache
fi
exit 0
endef

define Package/luci-app-chinadns/postrm
#!/bin/sh
rm -f /tmp/luci-indexcache
exit 0
endef

define Package/luci-app-chinadns/conffiles
/etc/config/chinadns
endef

define Package/luci-app-chinadns/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/chinadns.lua $(1)/usr/lib/lua/luci/controller/chinadns.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./luasrc/model/cbi/chinadns.lua $(1)/usr/lib/lua/luci/model/cbi/chinadns.lua
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/chinadns
	$(INSTALL_DATA) ./luasrc/view/chinadns/chinadns_status.htm $(1)/usr/lib/lua/luci/view/chinadns/chinadns_status.htm
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./root/etc/config/chinadns $(1)/etc/config/chinadns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/chinadns $(1)/etc/init.d/chinadns
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-chinadns $(1)/etc/uci-defaults/luci-chinadns
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/chinadns.po $(1)/usr/lib/lua/luci/i18n/chinadns.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-chinadns))
