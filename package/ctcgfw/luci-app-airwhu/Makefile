include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-airwhu
PKG_VERSION=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/luci-app-airwhu
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	DEPENDS:=+mentohust +kmod-ipt-nat6
	TITLE:=luci-app-airwhu
	PKGARCH:=all
endef

define Package/luci-app-airwhu/description
	 LuCI web-interface for Ruijie 802.1X Client with IPv6 NAT.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./* $(PKG_BUILD_DIR)/
	po2lmo $(PKG_BUILD_DIR)/po/airwhu.zh_Hans.po $(PKG_BUILD_DIR)/po/airwhu.zh-cn.lmo
endef  

define Build/Compile
endef

define Package/luci-app-airwhu/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n  
	
	$(INSTALL_CONF) ./files/root/etc/config/airwhu $(1)/etc/config/airwhu
	$(INSTALL_BIN) ./files/root/etc/init.d/mentohust $(1)/etc/init.d/mentohust
	$(INSTALL_BIN) ./files/root/etc/hotplug.d/iface/99-ipv6nat $(1)/etc/hotplug.d/iface/99-ipv6nat
	$(INSTALL_BIN) ./files/root/bin/ipv6masq.sh $(1)/bin/ipv6masq.sh
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/model/cbi/airwhu.lua $(1)/usr/lib/lua/luci/model/cbi/airwhu.lua
	$(INSTALL_DATA) ./files/root/usr/lib/lua/luci/controller/airwhu.lua $(1)/usr/lib/lua/luci/controller/airwhu.lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/po/airwhu.zh-cn.lmo $(1)/usr/lib/lua/luci/i18n/airwhu.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-airwhu))
