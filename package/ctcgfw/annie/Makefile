#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE.txt for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=annie
PKG_VERSION:=0.10.3
PKG_RELEASE:=2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/iawia002/annie.git
PKG_SOURCE_DATE:=2020-12-15
PKG_SOURCE_VERSION:=abc3c9df18173c91a5ca7a77fecc0665dea01aa1
PKG_MIRROR_HASH:=d49d79c342377a1edbf6870f7dd49cb4de70718934151793298004db72fc538a

PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS:= \
	CONFIG_ANNIE_COMPRESS_GOPROXY \
	CONFIG_ANNIE_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/iawia002/annie
GO_PKG_BUILD_PKG:=$$(GO_PKG)
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

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_ANNIE_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/annie
endif
endef

$(eval $(call GoBinPackage,annie))
$(eval $(call BuildPackage,annie))
