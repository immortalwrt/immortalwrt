# Copyright (C) 2020 Lienol
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=trojan-go
PKG_VERSION:=0.8.2
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/p4gefau1t/trojan-go.git
PKG_SOURCE_VERSION:=d051cf4c8852d708769ca1c4e514306a88da830b
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION)

PKG_CONFIG_DEPENDS := \
	CONFIG_TROJAN_GO_COMPRESS_GOPROXY \
	CONFIG_TROJAN_GO_COMPRESS_UPX

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/p4gefau1t/trojan-go
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	$(GO_PKG)/constant.Version=$(PKG_VERSION) \
	$(GO_PKG)/constant.Commit=$(PKG_SOURCE_VERSION)
GO_PKG_TAG:=-tags full

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	DEPENDS:=$$(GO_ARCH_DEPENDS)
	TITLE:=An unidentifiable mechanism that helps you bypass GFW. It's compatible with original trojan with experimental features.
	URL:=https://github.com/p4gefau1t/trojan-go
endef

define Package/$(PKG_NAME)/config

menu "Configuration"
	depends on PACKAGE_$(PKG_NAME)

config TROJAN_GO_COMPRESS_GOPROXY
	bool "Compiling with GOPROXY proxy"
	default n

config TROJAN_GO_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y

endmenu

endef

ifeq ($(CONFIG_TROJAN_GO_COMPRESS_GOPROXY),y)
export GO111MODULE=on
GOPROXY=https://goproxy.cn
#export GOPROXY=https://goproxy.io
#export GOPROXY=https://mirrors.aliyun.com/goproxy/
endif

define Build/Prepare
	tar -zxvf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components 1
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG))
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_TROJAN_GO_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/$(PKG_NAME)
endif
endef

define Package/$(PKG_NAME)/install
	$(call GoPackage/Package/Install/Bin,$(PKG_INSTALL_DIR))
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/$(PKG_NAME) $(1)/usr/bin/$(PKG_NAME)
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
