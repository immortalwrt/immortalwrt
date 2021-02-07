#
# Copyright (C) 2021 ImmortalWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=go-nats
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/enobufs/go-nats.git
PKG_SOURCE_DATE:=2019-09-23
PKG_SOURCE_VERSION:=4effc32e0f4006da0636b24f4468cf32ce494d71
PKG_MIRROR_HASH:=52156d096ba21fdc18316040749b20868e500d7bd785a0d33533610ba7f7169b

PKG_LICENSE:=MIT
PKG_LICENSE_FILE:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_GO_NATS_COMPRESS_GOPROXY \
	CONFIG_GO_NATS_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/enobufs/go-nats
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/go-nats
	SECTION:=net
	CATEGORY:=Network
	TITLE:=A NAT type discovery tool
	URL:=https://github.com/enobufs/go-nats
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/go-nats/description
	A NAT type discovery tool using STUN written purely in Go.
endef

define Package/go-nats/config
config GO_NATS_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config GO_NATS_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

ifeq ($(CONFIG_GO_NATS_COMPRESS_GOPROXY),y)
	export GO111MODULE=on
	export GOPROXY=https://goproxy.io
endif

define Build/Compile
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GO_NATS_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/go-nats
endif
endef

$(eval $(call GoBinPackage,go-nats))
$(eval $(call BuildPackage,go-nats))
