include $(TOPDIR)/rules.mk

PKG_NAME:=simple-torrent
PKG_VERSION:=1.2.12
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/boypt/simple-torrent.git
PKG_SOURCE_VERSION:=7d2662c35ac0a566b39b56c0d42de81067cc1670

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=AGPL-3.0
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=cloud-torrent
GO_PKG_LDFLAGS:=-s -w
GO_PKG_LDFLAGS_X:= \
	main.VERSION=$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=BitTorrent
	TITLE:=a self-hosted remote torrent client
	URL:=https://github.com/ginuerzh/gost
	DEPENDS:=$(GO_ARCH_DEPENDS)
endef

define Package/$(PKG_NAME)/description
	SimpleTorrent is a a self-hosted remote torrent client, written in Go (golang).
	Started torrents remotely, download sets of files on the local disk of the server,
	which are then retrievable or streamable via HTTP.
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
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/cloud-torrent
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/cloud-torrent $(1)/usr/bin/simple-torrent
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
