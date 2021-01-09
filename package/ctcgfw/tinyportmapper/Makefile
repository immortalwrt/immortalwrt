#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyPortMapper
PKG_VERSION:=20200818.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/tinyPortMapper.git
PKG_SOURCE_VERSION:=b6c5c05ba9a9db00571d185edcd534434f41b50a
PKG_MIRROR_HASH:=93946ec018ef515d0f443fd9a6e6510ce3ceb1be373a2393fe2daeeb5e7ced95

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=MIT
PKG_MAINTAINER:=Yu Wang

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

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

define Build/Prepare
	$(PKG_UNPACK)
	sed -i 's/cc_cross=.*/cc_cross=$(TARGET_CXX)/g' $(PKG_BUILD_DIR)/makefile
	sed -i '/\*gitversion/d' $(PKG_BUILD_DIR)/makefile
	echo 'const char *gitversion = "$(PKG_VERSION)";' > $(PKG_BUILD_DIR)/git_version.h
endef

define Package/tinyPortMapper/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinymapper_cross $(1)/usr/bin/tinymapper
endef

$(eval $(call BuildPackage,tinyPortMapper))
