include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-diskman
PKG_VERSION:=v0.2.3
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-diskman>
PKG_LICENSE:=AGPL-3.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/lisaac/luci-app-diskman.git
PKG_SOURCE_VERSION:=$(PKG_VERSION)

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

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
	tar -xzvf $(DL_DIR)/$(PKG_SOURCE) -C $(BUILD_DIR)
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/postinst
#!/bin/sh
rm -fr /tmp/luci-indexcache /tmp/luci-modulecache
endef

define Package/$(PKG_NAME)/install
	# $(INSTALL_DIR) $(1)/
	# cp -pR $(PKG_BUILD_DIR)/root/* $(1)/
	# $(INSTALL_DIR) $(1)/www
	# cp -pR $(PKG_BUILD_DIR)/htdoc/* $(1)/www
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci
	cp -pR $(PKG_BUILD_DIR)/luasrc/* $(1)/usr/lib/lua/luci/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	$$(foreach po,$$(shell find $(PKG_BUILD_DIR)/po/*/*.po), \
		po2lmo $$(po) \
		$(1)/usr/lib/lua/luci/i18n/diskman.$$(shell echo $$(po) | awk -F'/' '{print $$$$(NF-1)}').lmo;)
	#po2lmo $(PKG_BUILD_DIR)/po/zh-cn/diskman.po $(1)/usr/lib/lua/luci/i18n/diskman.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
