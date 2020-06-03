include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-dockerman
PKG_VERSION:=v0.5.13
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-dockerman>
PKG_LICENSE:=AGPL-3.0

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
endef

define Build/Compile
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
	cp -pR ./root/usr/* $(1)/usr/
	$(INSTALL_DIR) $(1)/etc/config
	cp -pR ./root/etc/config/* $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./root/etc/init.d/* $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./root/etc/uci-defaults/* $(1)/etc/uci-defaults/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci/
	$(INSTALL_DIR) $(1)/usr/share/rpcd/acl.d
	$(INSTALL_DATA) ./root/usr/share/rpcd/acl.d/*.json $(1)/usr/share/rpcd/acl.d/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh_Hans/dockerman.po $(1)/usr/lib/lua/luci/i18n/dockerman.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
