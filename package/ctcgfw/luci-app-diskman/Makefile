include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-diskman
PKG_VERSION:=v0.1.8
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-diskman>
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_LICENSE:=Apache-2.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Disk Manager interface for LuCI
	PKGARCH:=all
	DEPENDS:=+e2fsprogs +parted +smartmontools +blkid
endef

define Package/$(PKG_NAME)/description
	Disk Manager interface for LuCI
endef

define Build/Prepare
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -fr /tmp/luci-indexcache /tmp/luci-modulecache
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci
	# $(INSTALL_DIR) $(1)/
	# cp -pR ./root/* $(1)/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh-cn/diskman.po $(1)/usr/lib/lua/luci/i18n/diskman.zh-cn.lmo
	po2lmo ./po/pl/diskman.po $(1)/usr/lib/lua/luci/i18n/diskman.pl.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))