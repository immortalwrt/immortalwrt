#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v3.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=GoQuiet
PKG_VERSION:=1.2.2

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/cbeuw/GoQuiet.git
PKG_SOURCE_VERSION:=013cdfdf72000dcd4691799c37a0cf960ab4c82f
PKG_MIRROR_HASH:=c9c97463e5286e8ad10de5e86512b98f60672235c47cfd9696188a16496fc56c

PKG_LICENSE:=GPL-3.0
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:= \
	CONFIG_GQ_CLIENT_COMPRESS_UPX \
	CONFIG_GQ_SERVER_COMPRESS_UPX

GO_PKG:=github.com/cbeuw/GoQuiet
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:=main.version=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/GoQuiet/template
  SECTION:=net
  CATEGORY:=Network
  TITLE:=GoQuiet
  DEPENDS:=$(GO_ARCH_DEPENDS) +libpthread
  URL:=https://github.com/cbeuw/GoQuiet
  SUBMENU:=Web Servers/Proxies
endef

define Package/gq-client
  $(call Package/GoQuiet/template)
  TITLE+= Client
endef

define Package/gq-server
  $(call Package/GoQuiet/template)
  TITLE+= Server
endef

define Package/gq-client/config
config GQ_CLIENT_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

define Package/gq-server/config
config GQ_SERVER_COMPRESS_UPX
	bool "Compress executable files with UPX"
	default y
endef

GO_PKG_BUILD_VARS += GO111MODULE=auto

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/cmd/gq-client)
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GQ_CLIENT_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gq-client
endif
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/cmd/gq-server)
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
ifeq ($(CONFIG_GQ_SERVER_COMPRESS_UPX),y)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gq-server
endif
endef

define Package/gq-client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/gq-client $(1)/usr/bin/
endef

define Package/gq-server/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/gq-server $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,gq-client))
$(eval $(call GoBinPackage,gq-server))
$(eval $(call BuildPackage,gq-client))
$(eval $(call BuildPackage,gq-server))
