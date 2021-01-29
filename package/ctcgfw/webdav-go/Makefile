#
# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webdav-go
PKG_VERSION:=4.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/hacdias/webdav/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=3a6754933cc138623830944cb1ce9151baebe780b8111415c2e0044970586ca5

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_WEBDAV_GO_COMPRESS_GOPROXY \
	CONFIG_WEBDAV_GO_COMPRESS_UPX

PKG_BUILD_DIR:=$(BUILD_DIR)/$(firstword $(subst -, ,$(PKG_NAME)))-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/hacdias/webdav
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/webdav-go
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=File Transfer
	TITLE:=A powerful WebDav server written in Golang.
	URL:=https://github.com/hacdias/webdav
	DEPENDS:=$(GO_ARCH_DEPENDS) 
endef

define Package/webdav-go/description
  webdav command line interface is really easy to use so you can easily create a WebDAV server for your own user.
  By default, it runs on a random free port and supports JSON, YAML and TOML configuration.
endef

define Package/webdav-go/config
config WEBDAV_GO_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config WEBDAV_GO_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_WEBDAV_GO_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_WEBDAV_GO_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/webdav
endif
endef

define Package/webdav-go/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/webdav $(1)/usr/bin/webdav-go
endef

$(eval $(call GoBinPackage,webdav-go))
$(eval $(call BuildPackage,webdav-go))
