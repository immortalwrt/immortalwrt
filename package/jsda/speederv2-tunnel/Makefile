#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=speederv2-tunnel
PKG_VERSION:=20190121.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	TITLE:=speederv2
	DEPENDS:=
	URL:=https://github.com/wangyu-/UDPspeeder/releases
endef

define Package/$(PKG_NAME)/description
speederv2
endef

ifeq ($(ARCH),x86_64)
	PKG_ARCH_speederv2:=speederv2_amd64
endif

ifeq ($(ARCH),i386)
	PKG_ARCH_speederv2:=speederv2_x86
endif

define Build/Prepare
	[ ! -f $(PKG_BUILD_DIR)/speederv2_binaries.tar.gz ] && wget  https://github.com/wangyu-/UDPspeeder/releases/download/$(PKG_VERSION)/speederv2_binaries.tar.gz -O $(PKG_BUILD_DIR)/speederv2_binaries.tar.gz
	tar -xzvf $(PKG_BUILD_DIR)/speederv2_binaries.tar.gz -C $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_ARCH_speederv2) $(1)/usr/bin/speederv2
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
