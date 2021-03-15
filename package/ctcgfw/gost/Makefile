#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gost
PKG_VERSION:=2.11.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GHCODELOAD/ginuerzh/gost/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=d94b570a7a84094376b8c299d740528f51b540d9162f1db562247a15a89340bf

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_GOST_COMPRESS_GOPROXY \
	CONFIG_GOST_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/ginuerzh/gost
GO_PKG_BUILD_PKG:=github.com/ginuerzh/gost/cmd/gost
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/gost
	SECTION:=net
	CATEGORY:=Network
	TITLE:=GO Simple Tunnel
	URL:=https://github.com/ginuerzh/gost
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/gost/description
	A simple security tunnel written in Golang
endef

define Package/gost/config
config GOST_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config GOST_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_GOST_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GOST_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gost
endif
endef

define Package/gost/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gost $(1)/usr/bin/gost

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(CURDIR)/files/gost.config $(1)/etc/config/gost
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(CURDIR)/files/gost.init $(1)/etc/init.d/gost
endef

$(eval $(call GoBinPackage,gost))
$(eval $(call BuildPackage,gost))
