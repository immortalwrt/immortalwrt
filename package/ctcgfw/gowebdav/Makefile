include $(TOPDIR)/rules.mk

PKG_NAME:=gowebdav
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/1715173329/gowebdav.git
PKG_SOURCE_VERSION:=8b30e5453e959fd9911a90f2d4f7421616285ffa
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=gowebdav
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=File Transfer
	TITLE:=A simple WebDav server written in Golang.
	URL:=https://github.com/1715173329/gowebdav
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/$(PKG_NAME)/description
	A simple WebDav server written in Golang.
endef

define Build/Prepare
	tar -zxf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components 1
endef

define Build/Configure
endef

define Build/Compile
	$(eval GO_PKG_BUILD_PKG:=$(GO_PKG))
	$(call GoPackage/Build/Configure)
	$(call GoPackage/Build/Compile)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/gowebdav
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/gowebdav $(1)/usr/bin/gowebdav
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) $(CURDIR)/files/gowebdav.config $(1)/etc/config/gowebdav
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(CURDIR)/files/gowebdav.init $(1)/etc/init.d/gowebdav
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
