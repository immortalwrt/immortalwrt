#
# Copyright (C) 2016-2017 Jian Chang <aa65535@live.com>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-udp2raw
PKG_VERSION:=1.0.0
PKG_RELEASE:=5

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Jian Chang <aa65535@live.com>

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for udp2raw-tunnel
	PKGARCH:=all
	DEPENDS:=+udp2raw-tunnel
endef

define Package/$(PKG_NAME)/description
	LuCI Support for udp2raw-tunnel.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-udp2raw ) && rm -f /etc/uci-defaults/luci-udp2raw
fi
exit 0
endef

define Package/$(PKG_NAME)/conffiles
	/etc/config/udp2raw
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/udp2raw
	$(INSTALL_DATA) ./luasrc/model/cbi/udp2raw/*.lua $(1)/usr/lib/lua/luci/model/cbi/udp2raw/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/view/udp2raw
	$(INSTALL_DATA) ./luasrc/view/udp2raw/*.htm $(1)/usr/lib/lua/luci/view/udp2raw/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/udp2raw $(1)/etc/config/udp2raw
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/udp2raw $(1)/etc/init.d/udp2raw
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-udp2raw $(1)/etc/uci-defaults/luci-udp2raw
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/udp2raw.po $(1)/usr/lib/lua/luci/i18n/udp2raw.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
