#
# Copyright (C) 2021 Lean <coolsnowwolf@gmail.com>
#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=default-settings
PKG_VERSION:=1.1
PKG_RELEASE:=$(COMMITCOUNT)

PKG_LICENSE:=GPL-3.0

PKG_CONFIG_DEPENDS:=CONFIG_DEFAULT_SETTINGS_OPTIMIZE_FOR_CHINESE

include $(INCLUDE_DIR)/package.mk

define Package/default-settings
  SECTION:=luci
  CATEGORY:=LuCI
  TITLE:=LuCI support for Default Settings
  PKGARCH:=all
  DEPENDS:=+luci-base +@LUCI_LANG_en
endef

define Package/default-settings/config
config DEFAULT_SETTINGS_OPTIMIZE_FOR_CHINESE
	bool "Optimize for mainland China users"
	select LUCI_LANG_zh-cn
	default y
endef

define Package/default-settings/description
  Language Support Packages.
endef

define Build/Compile
endef

define Package/default-settings/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DATA) ./files/openwrt_banner $(1)/etc/openwrt_banner

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/zzz-default-settings $(1)/etc/uci-defaults/99-default-settings

ifeq ($(CONFIG_DEFAULT_SETTINGS_OPTIMIZE_FOR_CHINESE),y)
	$(INSTALL_BIN) ./files/zzz-default-settings-chinese $(1)/etc/uci-defaults/99-default-settings-chinese

	$(INSTALL_DIR) $(1)/usr/lib/lua/luci/i18n
	po2lmo ./i18n/default.zh-cn.po $(1)/usr/lib/lua/luci/i18n/default.zh-cn.lmo
	po2lmo ./i18n/more.zh-cn.po $(1)/usr/lib/lua/luci/i18n/more.zh-cn.lmo
endif
endef

$(eval $(call BuildPackage,default-settings))
