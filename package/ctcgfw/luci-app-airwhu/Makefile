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
endef  

define Build/Compile
endef

define Package/luci-app-airwhu/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/controller
	$(INSTALL_DATA) ./luasrc/controller/*.lua $(1)/usr/lib/lua/luci/controller/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/model/cbi
	$(INSTALL_DATA) ./luasrc/model/cbi/*.lua $(1)/usr/lib/lua/luci/model/cbi/
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./root/etc/config/airwhu $(1)/etc/config/airwhu
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/mentohust $(1)/etc/init.d/mentohust
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./root/etc/hotplug.d/iface/99-ipv6nat $(1)/etc/hotplug.d/iface/99-ipv6nat
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) ./root/bin/ipv6masq.sh $(1)/bin/ipv6masq.sh
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n  
	po2lmo ./po/zh_cn/airwhu.po $(1)/usr/lib/lua/luci/i18n/airwhu.zh-cn.lmo
endef

$(eval $(call BuildPackage,luci-app-airwhu))
