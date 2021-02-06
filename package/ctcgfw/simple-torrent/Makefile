#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=simple-torrent
PKG_VERSION:=1.2.12
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/boypt/simple-torrent/tar.gz/$(PKG_VERSION)?
PKG_HASH:=82cfa9cd9e9dca839ad542a24ecf67073fbcb703f9d890381148be99e8689716

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_SIMPLE_TORRENT_COMPRESS_GOPROXY \
	CONFIG_SIMPLE_TORRENT_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=cloud-torrent
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	main.VERSION=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/simple-torrent
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=BitTorrent
	TITLE:=A self-hosted remote torrent client
	URL:=https://github.com/ginuerzh/gost
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-bundle +libstdcpp
endef

define Package/simple-torrent/description
	SimpleTorrent is a a self-hosted remote torrent client, written in Go (golang).
	Started torrents remotely, download sets of files on the local disk of the server,
	which are then retrievable or streamable via HTTP.
endef

define Package/simple-torrent/config
config SIMPLE_TORRENT_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config SIMPLE_TORRENT_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_SIMPLE_TORRENT_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_SIMPLE_TORRENT_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/cloud-torrent
endif
endef

define Package/simple-torrent/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cloud-torrent $(1)/usr/bin/simple-torrent
endef

$(eval $(call GoBinPackage,simple-torrent))
$(eval $(call BuildPackage,simple-torrent))
