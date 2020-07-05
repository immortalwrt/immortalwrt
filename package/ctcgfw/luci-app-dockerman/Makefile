include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dockerman
PKG_VERSION:=v0.5.13
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-dockerman>
PKG_LICENSE:=AGPL-3.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lisaac/luci-app-dockerman.git
PKG_SOURCE_VERSION:=$(PKG_VERSION)

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_docker_ce
	bool "Include Docker-CE"
	default n
config PACKAGE_$(PKG_NAME)_INCLUDE_ttyd
	bool "Include ttyd"
	default y
endef

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Docker Manager interface for LuCI
	PKGARCH:=all
	DEPENDS:=+luci-lib-docker \
	+PACKAGE_$(PKG_NAME)_INCLUDE_docker_ce:docker-ce \
	+PACKAGE_$(PKG_NAME)_INCLUDE_ttyd:ttyd
endef

define Package/$(PKG_NAME)/description
	Docker Manager interface for LuCI
endef

define Build/Prepare
	tar -xzvf $(DL_DIR)/$(PKG_SOURCE) -C $(BUILD_DIR)
endef

define Build/Compile
endef

define Package/luci-app-dockerman/conffiles
/etc/config/dockerman
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
	( . /etc/uci-defaults/luci-app-dockerman ) && rm -f /etc/uci-defaults/luci-app-dockerman
fi
exit 0
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr
	cp -pR $(PKG_BUILD_DIR)/root/usr/* $(1)/usr/
	$(INSTALL_DIR) $(1)/etc/config
	cp -pR $(PKG_BUILD_DIR)/root/etc/config/* $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/root/etc/init.d/* $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/root/etc/uci-defaults/* $(1)/etc/uci-defaults/
	# $(INSTALL_DIR) $(1)/www
	# cp -pR $(PKG_BUILD_DIR)/htdoc/* $(1)/www
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$$(foreach po,$$(shell find $(PKG_BUILD_DIR)/po/*/*.po), \
		po2lmo $$(po) \
		$(1)/usr/lib/lua/luci/i18n/dockerman.$$(shell echo $$(po) | awk -F'/' '{print $$$$(NF-1)}').lmo;)
	#po2lmo $(PKG_BUILD_DIR)/po/zh_Hans/dockerman.po $(1)/usr/lib/lua/luci/i18n/dockerman.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
