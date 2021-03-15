#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyPortMapper
PKG_VERSION:=20200818.0

PKG_SOURCE_URL:=@GHCODELOAD/wangyu-/tinyPortMapper/tar.gz/$(PKG_VERSION)?
PKG_HASH:=094aef3fa0646fe3d0418f87767c1dd24ba1a80518f1e8a7cae2783aed88e732

PKG_LICENSE:=MIT
PKG_MAINTAINER:=Yu Wang

include $(INCLUDE_DIR)/package-immortalwrt.mk

define Package/tinyPortMapper
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A High-Performance Port Mapping/Forwarding Utility
  URL:=https://github.com/wangyu-/tinyPortMapper
  DEPENDS:=+libstdcpp +librt
endef

define Package/tinyPortMapper/description
  A Lightweight High-Performance Port Mapping/Forwarding Utility
  using epoll/libev, supports IPv4 and IPv6 for both TCP and UDP.
endef

MAKE_FLAGS += cross

define Build/Configure
	sed -i 's/cc_cross=.*/cc_cross=$(TARGET_CXX)/g' $(PKG_BUILD_DIR)/makefile
	sed -i '/\*gitversion/d' $(PKG_BUILD_DIR)/makefile
	echo 'const char *gitversion = "$(PKG_VERSION)";' > $(PKG_BUILD_DIR)/git_version.h
endef

define Package/tinyPortMapper/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinymapper_cross $(1)/usr/bin/tinymapper
endef

$(eval $(call BuildPackage,tinyPortMapper))
