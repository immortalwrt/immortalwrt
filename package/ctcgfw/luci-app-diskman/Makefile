include $(TOPDIR)/rules.mk

PKG_NAME:=luci-app-diskman
PKG_VERSION:=v0.2.10
PKG_RELEASE:=beta
PKG_MAINTAINER:=lisaac <https://github.com/lisaac/luci-app-diskman>
PKG_LICENSE:=AGPL-3.0

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/config
config PACKAGE_$(PKG_NAME)_INCLUDE_btrfs_progs
	bool "Include btrfs-progs"
	default n
config PACKAGE_$(PKG_NAME)_INCLUDE_lsblk
	bool "Include lsblk"
	default n
config PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include mdadm"
	default n
config PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456
	depends on PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include kmod-md-raid456"
	default n
config PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linear
	depends on PACKAGE_$(PKG_NAME)_INCLUDE_mdadm
	bool "Include kmod-md-linear"
	default n
endef

define Package/$(PKG_NAME)
	SECTION:=luci
	CATEGORY:=LuCI
	SUBMENU:=3. Applications
	TITLE:=Disk Manager interface for LuCI
	PKGARCH:=all
	DEPENDS:=+e2fsprogs +parted +smartmontools +blkid \
	+PACKAGE_$(PKG_NAME)_INCLUDE_btrfs_progs:btrfs-progs \
	+PACKAGE_$(PKG_NAME)_INCLUDE_lsblk:lsblk \
	+PACKAGE_$(PKG_NAME)_INCLUDE_mdadm:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_raid456:kmod-md-raid456 \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linears:mdadm \
	+PACKAGE_$(PKG_NAME)_INCLUDE_kmod_md_linears:kmod-md-linear
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
	cp -pR ./luasrc/* $(1)/usr/lib/lua/luci/
	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./po/zh_Hans/diskman.po $(1)/usr/lib/lua/luci/i18n/diskman.zh-cn.lmo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
