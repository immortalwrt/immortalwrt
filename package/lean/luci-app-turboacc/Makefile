# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the Apache License, Version 2.0 .
#

include $(TOPDIR)/rules.mk

LUCI_TITLE:=LuCI support for Flow Offload / Shortcut-FE
LUCI_DEPENDS:=+pdnsd-alt \
              +PACKAGE_luci-app-turboacc_INCLUDE_shortcut-fe:kmod-fast-classifier \
              +PACKAGE_luci-app-turboacc_INCLUDE_flow-offload:kmod-ipt-offload \
              +PACKAGE_luci-app-turboacc_INCLUDE_bbr-cca:kmod-tcp-bbr \
              +PACKAGE_luci-app-turboacc_INCLUDE_adguardhome:AdGuardHome \
              +PACKAGE_luci-app-turboacc_INCLUDE_dnsforwarder:dnsforwarder
LUCI_PKGARCH:=all

PKG_NAME:=luci-app-turboacc
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_LICENSE:=GPLv3.0+

define Package/luci-app-turboacc/config
config PACKAGE_luci-app-turboacc_INCLUDE_shortcut-fe
	bool "Include Shortcut-FE"
	depends on PACKAGE_luci-app-turboacc_INCLUDE_flow-offload=n
	default n

config PACKAGE_luci-app-turboacc_INCLUDE_flow-offload
	bool "Include Flow Offload"
	default y

config PACKAGE_luci-app-turboacc_INCLUDE_bbr-cca
	bool "Include BBR CCA"
	default y

config PACKAGE_luci-app-turboacc_INCLUDE_adguardhome
	bool "Include AdGuardHome"
	default n

config PACKAGE_luci-app-turboacc_INCLUDE_dnsforwarder
	bool "Include DNSForwarder"
	default n
endef
include $(TOPDIR)/feeds/luci/luci.mk

# call BuildPackage - OpenWrt buildroot signature
