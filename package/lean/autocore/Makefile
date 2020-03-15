#
# Copyright (C) 2010-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=autocore
PKG_VERSION:=1
PKG_RELEASE:=26


include $(INCLUDE_DIR)/package.mk

define Package/autocore
  TITLE:=x86/x64 auto core loadbalance script.
  MAINTAINER:=Lean
  DEPENDS:=@TARGET_x86 +bc +lm-sensors +ethtool
endef

define Package/autocore/description
A usb autoconfig hotplug script.
endef

define Build/Compile
endef

define Package/autocore/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/rpcd_luci $(1)/etc/rpcd_luci
	$(INSTALL_DATA) ./files/rpcd_luci-base.json $(1)/etc/rpcd_luci-base.json
	$(INSTALL_DATA) ./files/rpcd_10_system.js $(1)/etc/rpcd_10_system.js
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/autocore $(1)/etc/init.d/autocore
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/cpuinfo $(1)/sbin/cpuinfo
	$(INSTALL_BIN) ./files/ethinfo $(1)/sbin/ethinfo
	$(INSTALL_DIR) $(1)/www/luci-static/resources/view/status/include
	$(INSTALL_DATA) ./files/rpcd_21_ethinfo.js $(1)/www/luci-static/resources/view/status/include/21_ethinfo.js
endef

$(eval $(call BuildPackage,autocore))
