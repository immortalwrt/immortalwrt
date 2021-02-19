#
# Copyright (c) 2017 Yu Wang <wangyucn@gmail.com>
#
# This is free software, licensed under the MIT.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tinyfecVPN
PKG_VERSION:=20210116.0
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/wangyu-/tinyfecVPN.git
PKG_SOURCE_DATE:=2021-01-17
PKG_SOURCE_VERSION:=c9b98721ea1d62ab8e1c4c9e2a0ed04394dfe0ae
PKG_MIRROR_HASH:=28cdadf0f162e52d3c6b10b6a6cdcd22e80783b36b33e341b5dea156bfb75b74

PKG_LICENSE:=MIT
PKG_MAINTAINER:=Yu Wang

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/tinyfecvpn
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=VPN
  TITLE:=A Network Improving Tool
  URL:=https://github.com/wangyu-/tinyfecVPN
  DEPENDS:=+libpthread +libstdcpp +kmod-tun
endef

define Package/tinyfecvpn/description
  A Lightweight VPN with Build-in Forward Error Correction Support (or A Network Improving Tool
  which works at VPN mode). Improves your Network Quality on a High-latency Lossy Link.
endef

MAKE_FLAGS += cross

define Build/Configure
	sed -i 's/cc_cross=.*/cc_cross=$(TARGET_CXX)/g' $(PKG_BUILD_DIR)/makefile
	sed -i '/\*gitversion/d' $(PKG_BUILD_DIR)/makefile
	echo 'const char *gitversion = "$(PKG_SOURCE_VERSION)";' > $(PKG_BUILD_DIR)/git_version.h

	$(call Build/Configure/Default)
endef

define Package/tinyfecvpn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinyvpn_cross $(1)/usr/bin/tinyvpn
endef

$(eval $(call BuildPackage,tinyfecvpn))
