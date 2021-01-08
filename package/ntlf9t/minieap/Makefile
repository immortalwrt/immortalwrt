#
# Copyright (C) 2016 Openwrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minieap
PKG_VERSION:=0.93
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/updateing/minieap.git
PKG_SOURCE_VERSION:=9e10f320a9fce46c8f4286324a0fc70e572f61dc
PKG_MIRROR_HASH:=41c976810c994d6125f6fe3cb1d978f7e257aa587ee32229364942a82621b273

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

include $(INCLUDE_DIR)/package.mk

define Package/minieap
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Extendable 802.1x client
	URL:=https://github.com/updateing/minieap
	SUBMENU:=Campus Network
endef

define Package/minieap/description
Extendable 802.1x client with Ruijie v3 (v4) plugin.
endef

define Package/minieap/conffiles
/etc/minieap.conf
endef

define Package/minieap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/minieap $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,minieap))
