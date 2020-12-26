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
PKG_MIRROR_HASH:=d16557d89ac909877128729590e34a3c7a9b430000525ed133e2fffb56a0310a
PKG_SOURCE_VERSION:=013cdfdf72000dcd4691799c37a0cf960ab4c82f

PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/cbeuw/GoQuiet
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/gq-client
  SECTION:=net
  CATEGORY:=Network
  TITLE:=GoQuiet Client
  DEPENDS:=$(GO_ARCH_DEPENDS) +libpthread
  URL:=https://github.com/cbeuw/GoQuiet
  SUBMENU:=Web Servers/Proxies
endef

define Package/gq-server
  SECTION:=net
  CATEGORY:=Network
  TITLE:=GoQuiet Server
  DEPENDS:=$(GO_ARCH_DEPENDS) +libpthread
  URL:=https://github.com/cbeuw/GoQuiet
  SUBMENU:=Web Servers/Proxies
endef

define Package/gq-client/description
GoQuiet-Client
endef

define Package/gq-server/description
GoQuiet-Server
endef

define Build/Prepare
	tar -zxf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components 1
endef

define Build/Configure
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/cmd/gq-client)
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gq-client
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG)/cmd/gq-server)
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gq-server
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
