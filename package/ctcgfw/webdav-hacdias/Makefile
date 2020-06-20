
include $(TOPDIR)/rules.mk

PKG_NAME:=webdav-hacdias
PKG_VERSION:=3.0.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/hacdias/webdav.git
PKG_SOURCE_DATE:=2019-11-10
PKG_SOURCE_VERSION:=72d8e39927e5f2187949d5b6449e2f860a7fe223
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MIRROR_HASH:=5587cf5bddfb5f8b57f8ad23e521ddfd6c623ebb2f7381555c3f20d7f147d7e8

PKG_LICENSE:=GPLv3
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DEPENDS:=golang/host
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0 # https://github.com/openwrt/packages/issues/8498
GO_PKG:=github.com/hacdias/webdav
GO_PKG_LDFLAGS:=-s -w

include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/golang/golang-package.mk

define Package/$(PKG_NAME)
	SECTION:=net
        CATEGORY:=Network
        SUBMENU:=File Transfer
	TITLE:=A powerful WebDav server written in Golang.
	URL:=https://github.com/hacdias/webdav
	DEPENDS:=$(GO_ARCH_DEPENDS) 
endef

define Package/$(PKG_NAME)/description
webdav command line interface is really easy to use so you can easily create a WebDAV server for your own user.
By default, it runs on a random free port and supports JSON, YAML and TOML configuration.
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(GO_PKG_BUILD_BIN_DIR)/webdav $(1)/usr/bin/
endef

$(eval $(call GoBinPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
