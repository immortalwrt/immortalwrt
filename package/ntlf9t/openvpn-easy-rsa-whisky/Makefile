#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn-easy-rsa-whisky

PKG_VERSION:=3.0.6
PKG_RELEASE:=2
PKG_SOURCE_URL:=https://github.com/OpenVPN/easy-rsa/releases/download/v$(PKG_VERSION)/
PKG_SOURCE:=EasyRSA-unix-v$(PKG_VERSION).tgz
PKG_HASH:=cb29aed2d27824e59dbaad547f11dcab380a53c9fe05681249e804af436f1396
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR:=$(BUILD_DIR)/EasyRSA-v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/openvpn-easy-rsa-whisky
  TITLE:=CLI utility to build and manage a PKI CA.
  SECTION:=net
  CATEGORY:=Network
  URL:=http://openvpn.net
  SUBMENU:=VPN
  DEPENDS:=+openssl-util
  PKGARCH:=all
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/openvpn-easy-rsa-whisky/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/easyrsa $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/easy-rsa
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/openssl-easyrsa.cnf $(1)/etc/easy-rsa/openssl-easyrsa.cnf
	$(INSTALL_DIR) $(1)/etc/easy-rsa/x509-types
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/x509-types/* $(1)/etc/easy-rsa/x509-types/
endef

$(eval $(call BuildPackage,openvpn-easy-rsa-whisky))
