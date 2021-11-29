# SPDX-License-Identifier: GPL-3.0-only
#
# Copyright (C) 2020 Lean <coolsnowwolf@gmail.com>
# Copyright (C) 2021 ImmortalWrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_VERSION:=2
PKG_RELEASE:=$(COMMITCOUNT)

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_bcm27xx \
	CONFIG_TARGET_bcm53xx

include $(INCLUDE_DIR)/package.mk

define Package/autocore-arm
  TITLE:=ARM auto core script.
  MAINTAINER:=CN_SZTL
  DEPENDS:=@(arm||aarch64) \
    +TARGET_bcm27xx:bcm27xx-userland \
    +TARGET_bcm53xx:nvram
  VARIANT:=arm
endef

define Package/autocore-x86
  TITLE:=x86/x64 auto core loadbalance script.
  MAINTAINER:=Lean / CN_SZTL
  DEPENDS:=@TARGET_x86 +bc +lm-sensors +ethtool
  VARIANT:=x86
endef

define Package/autocore-arm/description
  Display more details info about the devices in LuCI.
endef

define Package/autocore-x86/description
  A USB autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/autocore-arm/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/arm/rpcd_luci $(1)/etc/rpcd_luci
	$(INSTALL_DATA) ./files/arm/rpcd_luci-mod-status.json $(1)/etc/rpcd_luci-mod-status.json
	$(INSTALL_DATA) ./files/arm/rpcd_10_system.js $(1)/etc/rpcd_10_system.js

	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/arm/090-cover-index_files $(1)/etc/uci-defaults/090-cover-index_files

	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/arm/cpuinfo $(1)/sbin/cpuinfo
endef

define Package/autocore-x86/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/x86/rpcd_luci $(1)/etc/rpcd_luci
	$(INSTALL_DATA) ./files/x86/rpcd_luci-mod-status.json $(1)/etc/rpcd_luci-mod-status.json
	$(INSTALL_DATA) ./files/x86/rpcd_10_system.js $(1)/etc/rpcd_10_system.js

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/x86/autocore $(1)/etc/init.d/autocore

	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/x86/cpuinfo $(1)/sbin/cpuinfo
	$(INSTALL_BIN) ./files/x86/ethinfo $(1)/sbin/ethinfo

	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/status/include
	$(INSTALL_DATA) ./files/x86/rpcd_21_ethinfo.js $(1)/www/luci-static/resources/view/status/include/21_ethinfo.js
endef

$(eval $(call BuildPackage,autocore-arm))
$(eval $(call BuildPackage,autocore-x86))
