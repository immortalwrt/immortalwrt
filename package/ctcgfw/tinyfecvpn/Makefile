#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyfecVPN
PKG_VERSION:=20180820.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/tinyfecVPN.git
PKG_SOURCE_VERSION:=e8364ba8ff94415ee3bedcbdb9557bfe5994a469
PKG_MIRROR_HASH:=30fd808b0203c70e8e6cfe8ca55b907faaea6a8be12a7a7fef6658f4ddbb523c

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=MIT
PKG_MAINTAINER:=Yu Wang

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tinyfecVPN
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=A Network Improving Tool
  URL:=https://github.com/wangyu-/tinyfecVPN
  DEPENDS:=+libpthread +libstdcpp +kmod-tun
endef

define Package/tinyfecVPN/description
  A Lightweight VPN with Build-in Forward Error Correction Support (or A Network Improving Tool
  which works at VPN mode). Improves your Network Quality on a High-latency Lossy Link.
endef

MAKE_FLAGS += cross

define Build/Prepare
	$(PKG_UNPACK)
	sed -i 's/cc_cross=.*/cc_cross=$(TARGET_CXX)/g' $(PKG_BUILD_DIR)/makefile
	sed -i '/\*gitversion/d' $(PKG_BUILD_DIR)/makefile
	echo 'const char *gitversion = "$(PKG_VERSION)";' > $(PKG_BUILD_DIR)/git_version.h
endef

define Package/tinyfecVPN/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinyvpn_cross $(1)/usr/bin/tinyvpn
endef

$(eval $(call BuildPackage,tinyfecVPN))
