#
# Copyright (C) 2015 OpenWrt-dist
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipv6helper
PKG_VERSION:=1.0
PKG_RELEASE:=4

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=LEAN

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/ipv6helper
	SECTION:=ipv6
	TITLE:=IPv6 Helper and Dynamic Update he.net of ip
	DEPENDS:=+luci-proto-ipv6 +wget-ssl +libip6tc +ip6tables +ip6tables-mod-nat \
		+kmod-ipt-nat6 +kmod-nf-nat6 +odhcpd-ipv6only +odhcp6c +6in4
	PKGARCH:=all
endef

define Package/ipv6helper/description
	IPv6 Helper and Dynamic Update he.net of ip
endef

define Build/Compile
	true
endef

define Package/ipv6helper/install
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./files/60-6in4 $(1)/etc/hotplug.d/iface/60-6in4
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/60-ipv6-hybrid $(1)/etc/uci-defaults/60-ipv6-hybrid
endef

$(eval $(call BuildPackage,ipv6helper))
