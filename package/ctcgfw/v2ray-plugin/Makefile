#
# Copyright (C) 2020 SharerMax
#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=v2ray-plugin
PKG_VERSION:=4.35.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/teddysun/v2ray-plugin/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=c0230e0ca2872f9644460d637a5e61d3c7ad84be811af8cbe301858a93839000

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=madeye <max.c.lv@gmail.com>

PKG_CONFIG_DEPENDS:= \
	CONFIG_V2RAY_PLUGIN_COMPRESS_GOPROXY \
	CONFIG_V2RAY_PLUGIN_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/shadowsocks/v2ray-plugin
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.VERSION=v$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/v2ray-plugin/config
config V2RAY_PLUGIN_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config V2RAY_PLUGIN_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_GOWEBDAV_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Package/v2ray-plugin
	SECTION:=net
	CATEGORY:=Network
	TITLE:=SIP003 plugin for shadowsocks, based on v2ray
	URL:=https://github.com/teddysun/v2ray-plugin
	DEPENDS:=$(GO_ARCH_DEPENDS) +ca-certificates
endef

define Package/v2ray-plugin/description
	Yet another SIP003 plugin for shadowsocks, based on v2ray
endef

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_V2RAY_PLUGIN_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/v2ray-plugin
endif
endef

$(eval $(call GoBinPackage,v2ray-plugin))
$(eval $(call BuildPackage,v2ray-plugin))
