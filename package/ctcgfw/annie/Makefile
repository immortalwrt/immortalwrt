#
# Copyright (C) 2020 Project OpenWrt
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE.txt for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=annie
PKG_VERSION:=0.10.3-67298d5
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/iawia002/annie.git
PKG_SOURCE_VERSION:=67298d55aaf43d840b8d7283b0ac99736173d285
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS := \
	CONFIG_ANNIE_COMPRESS_GOPROXY \
	CONFIG_ANNIE_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/iawia002/annie
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/annie
	SECTION:=multimedia
	CATEGORY:=Multimedia
	TITLE:=Fast, simple and clean video downloader
	URL:=https://github.com/iawia002/annie
	DEPENDS:=$(GO_ARCH_DEPENDS) +ffmpeg
endef

define Package/annie/description
	Annie is a fast, simple and clean video downloader built with Go.
endef

define Package/annie/config
config ANNIE_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config ANNIE_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_ANNIE_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Prepare
	tar -zxf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components 1
endef

define Build/Configure
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG))
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_ANNIE_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/annie
endif
endef

define Package/annie/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/annie $(1)/usr/bin/annie
endef

$(eval $(call GoBinPackage,annie))
$(eval $(call BuildPackage,annie))
