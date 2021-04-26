#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=UnblockNeteaseMusic-Go
PKG_VERSION:=0.2.10
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/cnsilvan/UnblockNeteaseMusic/tar.gz/$(PKG_VERSION)?
PKG_HASH:=2103562e399d04fcce0aa017057357e6fcec8beb6dc65c17ad1bdc06f38d6b7a

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=Silvan <cnsilvan@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_UNBLOCKNETEASEMUSIC_GO_COMPRESS_GOPROXY \
	CONFIG_UNBLOCKNETEASEMUSIC_GO_COMPRESS_UPX

PKG_BUILD_DIR:=$(BUILD_DIR)/$(firstword $(subst -, ,$(PKG_NAME)))-$(PKG_VERSION)
PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/cnsilvan/UnblockNeteaseMusic
GO_PKG_BUILD_PKG:=$$(GO_PKG)
GO_PKG_LDFLAGS:=-s -w
COMPILE_TIME:= $(shell TZ=UTC-8 date '+%Y-%m-%d %H:%M:%S')
GO_PKG_LDFLAGS+= \
	-X '$(GO_PKG)/version.Version=$(PKG_VERSION)' \
	-X '$(GO_PKG)/version.BuildTime=$(COMPILE_TIME)' \
	-X '$(GO_PKG)/version.ExGoVersionInfo=$(GO_ARM) $(GO_MIPS)$(GO_MIPS64)'

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/UnblockNeteaseMusic-Go/config
config UNBLOCKNETEASEMUSIC_GO_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config UNBLOCKNETEASEMUSIC_GO_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_UNBLOCKNETEASEMUSIC_GO_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Package/UnblockNeteaseMusic-Go
	SECTION:=multimedia
	CATEGORY:=Multimedia
	TITLE:=Revive Netease Cloud Music (Golang)
	URL:=https://github.com/cnsilvan/UnblockNeteaseMusic
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GOST_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/UnblockNeteaseMusic
endif
endef

define Package/UnblockNeteaseMusic-Go/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/UnblockNeteaseMusic $(1)/usr/bin/UnblockNeteaseMusic

	$(INSTALL_DIR) $(1)/usr/share/UnblockNeteaseMusicGo
	$(CP) ./files/* $(1)/usr/share/UnblockNeteaseMusicGo/
	
endef

$(eval $(call GoBinPackage,UnblockNeteaseMusic-Go))
$(eval $(call BuildPackage,UnblockNeteaseMusic-Go))
