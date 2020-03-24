include $(TOPDIR)/rules.mk

PKG_NAME:=syncthing
PKG_VERSION:=1.4.0
PKG_RELEASE:=2

PKG_SOURCE:=syncthing-source-v$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/syncthing/syncthing/releases/download/v$(PKG_VERSION)
PKG_HASH:=0c45955445752dac43d56bc321fae2140b5b05ad6d41a574ed37813607493edd

PKG_BUILD_DIR=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)

PKG_MAINTAINER:=Paul Spooren <mail@aparcar.org>
PKG_LICENSE:=MPL-2.0
PKG_LICENSE_FILES:=LICENSE
PKG_CPE_ID:=cpe:/a:syncthing:syncthing

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

GO_PKG:=github.com/syncthing/syncthing/
GO_PKG_BUILD_PKG:=github.com/syncthing/syncthing/cmd/syncthing/
GO_PKG_INSTALL_EXTRA:=^gui/

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/syncthing
  TITLE:=Continuous file synchronization program
  URL:=https://syncthing.net
  DEPENDS:=$(GO_ARCH_DEPENDS)
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=File Transfer
endef

GO_PKG_LDFLAGS:= -s -w
GO_PKG_LDFLAGS_X:= \
	github.com/syncthing/syncthing/lib/build.Version=v$(PKG_VERSION) \
	github.com/syncthing/syncthing/lib/build.Stamp=$(SOURCE_DATE_EPOCH) \
	github.com/syncthing/syncthing/lib/build.User=ctcgfw \
	github.com/syncthing/syncthing/lib/build.Host=project-openwrt \
	github.com/syncthing/syncthing/lib/build.Program=syncthing

define Build/Compile
	$(call GoPackage/Build/Compile,-tags noupgrade)
	$(STAGING_DIR_HOST)/bin/upx --lzma --best $(GO_PKG_BUILD_BIN_DIR)/syncthing
endef

define Package/syncthing/description
		Syncthing replaces proprietary sync and cloud services with something
		open, trustworthy and decentralized. Your data is your data alone and
		you deserve to choose where it is stored, if it is shared with some
		third party and how it's transmitted over the Internet.
endef

define Package/syncthing/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/syncthing $(1)/usr/bin/syncthing
endef

$(eval $(call GoBinPackage,syncthing))
$(eval $(call BuildPackage,syncthing))
