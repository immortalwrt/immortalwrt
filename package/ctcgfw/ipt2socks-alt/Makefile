#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ipt2socks-alt
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/zfl9/ipt2socks/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=f247d72b61b1f40ce347904c43523bff257fc9f00ff269ecccf06f1d0f4b9d5e

PKG_BUILD_DIR:=$(BUILD_DIR)/$(firstword $(subst -, ,$(PKG_NAME)))-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILE:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/ipt2socks-alt
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Utility for converting iptables (REDIRECT/TPROXY) to SOCKS5
	URL:=https://github.com/zfl9/ipt2socks
	DEPENDS:=+libpthread +libuv
endef

TARGET_CFLAGS += $(FPIC) -flto
TARGET_LDFLAGS += -flto

define Package/ipt2socks-alt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/ipt2socks $(1)/usr/bin/ipt2socks-alt
endef

$(eval $(call BuildPackage,ipt2socks-alt))
