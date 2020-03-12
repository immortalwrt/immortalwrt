#
# Copyright (C) 2018 chenhw2 <https://github.com/chenhw2/>
#
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-aliddns
PKG_VERSION:=0.3.0
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=chenhw2 <https://github.com/chenhw2/>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-aliddns
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=LuCI Support for aliddns
	PKGARCH:=all
	DEPENDS:=+openssl-util +curl
endef

define Package/luci-app-aliddns/description
	LuCI Support for ALiDDNS.
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/luci-app-aliddns/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	if [ -f /etc/uci-defaults/luci-aliddns ]; then
		( . /etc/uci-defaults/luci-aliddns ) && \
		rm -f /etc/uci-defaults/luci-aliddns
	fi
	rm -rf /tmp/luci-indexcache /tmp/luci-modulecache
fi
exit 0
endef

define Package/luci-app-aliddns/prerm
#!/bin/sh
/etc/init.d/aliddns stop
exit 0
endef

define Package/luci-app-aliddns/conffiles
/etc/config/aliddns
endef

define Package/luci-app-aliddns/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi/aliddns
	$(INSTALL_DATA) ./luasrc/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./root/etc/config/aliddns $(1)/etc/config/aliddns
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/aliddns $(1)/etc/init.d/aliddns
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/luci-aliddns $(1)/etc/uci-defaults/luci-aliddns
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./root/usr/sbin/aliddns $(1)/usr/sbin/aliddns
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/aliddns.po $(1)/usr/lib/lua/luci/i18n/aliddns.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-aliddns))
