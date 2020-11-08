# Copyright (C) 2020 [CTCGFW] Project OpenWrt
#
# This is a free software, use it under GNU General Public License v3.0.

include $(TOPDIR)/rules.mk

PKG_NAME:=yq
PKG_VERSION:=3.4.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/mikefarah/yq/tar.gz/$(PKG_VERSION)?
PKG_HASH:=73259f808d589d11ea7a18e4cd38a2e98b518a6c2c178d1ec57d9c5942277cb1

PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1

PKG_CONFIG_DEPENDS:= \
	CONFIG_YQ_COMPRESS_GOPROXY \
	CONFIG_YQ_COMPRESS_UPX

GO_PKG:=github.com/mikefarah/yq

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/yq
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=yq is a portable command-line YAML processor
  URL:=https://mikefarah.gitbook.io/yq/
  DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/yq/description
  The aim of the project is to be the jq or sed of yaml files.
endef

define Package/yq/config
config YQ_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config YQ_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_YQ_COMPRESS_GOPROXY),y)
export GO111MODULE=on
export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_YQ_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/yq
endif
endef

define Package/yq/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/yq $(1)/usr/bin/
endef

$(eval $(call BuildPackage,yq))
