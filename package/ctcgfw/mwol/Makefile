#
# Copyright (C) 2019 mleaf.org
# 微信公众号【WiFi物联网】
#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mwol
PKG_VERSION:=1.0.0
PKG_RELEASE=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Mleaf/mwol.git
PKG_SOURCE_DATE:=2020-01-08
PKG_SOURCE_VERSION:=59f9805901b4ac2916a0273ffbc29197fcd17a62
PKG_MIRROR_HASH:=64f88b81ed88b1695f240fe4c0414bd32a5cf6e53ea008c67713ae8315ff3d0d

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=COPYING
PKG_MAINTAINER:=mleaf <mleaf90@gmail.com>

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/mwol
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libpthread +libmosquitto-ssl +luci-lib-json +libopenssl
  TITLE:=MQTT Wake On Lan
  URL:=http://www.mleaf.org
endef

define Package/mwol/description
	mwol is a project that uses mqtt to implement reverse proxy and wake up the computer.
endef

define Package/mwol/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mwol $(1)/usr/sbin/mwol

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) files/mwol.config $(1)/etc/config/mwol
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) files/mwol.init $(1)/etc/init.d/mwol

	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) files/mwol_config_json $(1)/usr/sbin/mwol_config_json
	$(INSTALL_DIR) $(1)/usr/share/mwol/ssl
	$(CP) files/ssl/* $(1)/usr/share/mwol/ssl/
endef

$(eval $(call BuildPackage,mwol))
